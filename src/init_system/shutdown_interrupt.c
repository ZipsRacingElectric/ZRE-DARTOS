// For asprintf. Note this must be the first include in this file.
#define _GNU_SOURCE
#include <stdio.h>

// Header
#include "shutdown_interrupt.h"

// Includes
#include "init_system_stdio.h"

// C Standard Library
#include <stdlib.h>

// Version 1.x.x implementation
#if LIBGPIOD_VERSION_MAJOR == 1

struct shutdownInterrupt
{
	struct gpiod_chip* chip;
	struct gpiod_line* line;
};

#else // Version 2.x.x or later

struct shutdownInterrupt
{
	struct gpiod_chip* chip;
	struct gpiod_line_request* lineRequest;
	unsigned int offsets [1];
};

#endif

shutdownInterrupt_t* shutdownInterruptInit (const char* consumer, const char* chipName, unsigned int lineNumber)
{
	// Allocate the interrupt structure. Not done statically, as the size of the structure depends on the library version.
	shutdownInterrupt_t* interrupt = malloc (sizeof (shutdownInterrupt_t));
	if (interrupt == NULL)
		return NULL;

	// Version 1.x.x implementation
	#if LIBGPIOD_VERSION_MAJOR == 1

	// Open the GPIO chip
	interrupt->chip = gpiod_chip_open_by_name (chipName);
	if (interrupt->chip == NULL)
	{
		int code = errno;
		fprintf (stderr, STDIO_PREFIX "Failed to open GPIO chip '%s': %s.\n", chipName, strerror (code));
		return code;
	}

	// Request control of the GPIO line.
	interrupt->line = gpiod_chip_get_line (interrupt->chip, lineNumber);
	if (interrupt->line == NULL)
	{
		int code = errno;
		fprintf (stderr, STDIO_PREFIX "Failed to get GPIO line %u: %s.\n", lineNumber, strerror (code));
		gpiod_chip_close (interrupt->chip);
		return code;
	}

	// Request notification of the falling edge event. Also set the GPIO to use a pullup resistor.
	int code = gpiod_line_request_falling_edge_events_flags (interrupt->line, consumer, GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);
	if (code < 0)
	{
		perror (STDIO_PREFIX "Failed to request GPIO line for falling edge notification");
		gpiod_line_release (interrupt->line);
		gpiod_chip_close (interrupt->chip);
		return code;
	}

	return interrupt;

	#else // Version 2.x.x or later

	// Convert the chip name into a device path. Assuming we are always working with devices in /dev/, which is true to the
	// RPi.
	char* path;
	if (asprintf (&path, "/dev/%s", chipName) < 0)
		return NULL;

	// Open the GPIO chip
	interrupt->chip = gpiod_chip_open (path);
	if (interrupt->chip == NULL)
	{
		fprintf (stderr, STDIO_PREFIX "Failed to open GPIO chip '%s': %s.\n", path, strerror (errno));
		free (path);
		return NULL;
	}

	// Release the allocated string
	free (path);

	// Create the request config using the provided consumer and default buffer size.
	struct gpiod_request_config* requestConfig = gpiod_request_config_new ();
	if (requestConfig == NULL)
	{
		perror (STDIO_PREFIX "Failed to create GPIO request config");
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}
	gpiod_request_config_set_consumer (requestConfig, consumer);
	gpiod_request_config_set_event_buffer_size (requestConfig, 0);

	// Create the line settings.
	struct gpiod_line_settings* lineSettings = gpiod_line_settings_new ();
	if (lineSettings == NULL)
	{
		perror (STDIO_PREFIX "Failed to create GPIO line settings");
		gpiod_request_config_free (requestConfig);
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}

	// Set the GPIO to act as an input.
	if (gpiod_line_settings_set_direction (lineSettings, GPIOD_LINE_DIRECTION_INPUT) != 0)
	{
		perror (STDIO_PREFIX "Failed to set GPIO line direction to input");
		gpiod_request_config_free (requestConfig);
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}

	// Set the GPIO to be biased with a pullup resistor.
	if (gpiod_line_settings_set_bias (lineSettings, GPIOD_LINE_BIAS_PULL_UP) != 0)
	{
		perror (STDIO_PREFIX "Failed to set GPIO line bias to pullup");
		gpiod_request_config_free (requestConfig);
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}

	// Set the GPIO to use falling edge detection.
	if (gpiod_line_settings_set_edge_detection (lineSettings, GPIOD_LINE_EDGE_FALLING) != 0)
	{
		perror (STDIO_PREFIX "Failed to set GPIO edge detection");
		gpiod_request_config_free (requestConfig);
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}

	// Create the GPIO line config.
	struct gpiod_line_config* lineConfig = gpiod_line_config_new ();
	if (lineConfig == NULL)
	{
		perror (STDIO_PREFIX "Failed to create GPIO line config");
		gpiod_request_config_free (requestConfig);
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}

	// Offset array contains the array of lines to request. Here we only want 1.
	interrupt->offsets [0] = lineNumber;

	// Add the GPIO line settings to the GPIO line config
	if (gpiod_line_config_add_line_settings (lineConfig, interrupt->offsets, sizeof (interrupt->offsets) / sizeof (unsigned int), lineSettings) != 0)
	{
		perror (STDIO_PREFIX "Failed to add GPIO line settings to GPIO line config");
		gpiod_line_config_free (lineConfig);
		gpiod_request_config_free (requestConfig);
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}

	// Request the GPIO line
	interrupt->lineRequest = gpiod_chip_request_lines (interrupt->chip, requestConfig, lineConfig);
	if (interrupt->lineRequest == NULL)
	{
		perror (STDIO_PREFIX "Failed to request GPIO line");
		gpiod_line_config_free (lineConfig);
		gpiod_request_config_free (requestConfig);
		gpiod_chip_close (interrupt->chip);
		return NULL;
	}

	// Release the line config and request config structures.
	gpiod_line_config_free (lineConfig);
	gpiod_request_config_free (requestConfig);

	return interrupt;

	#endif
}

int shutdownInterruptPoll (shutdownInterrupt_t* interrupt)
{
	// Version 1.x.x implementation
	#if LIBGPIOD_VERSION_MAJOR == 1

	while (true)
	{
		// Wait for the falling edge event.
		// - Note I do not know how to use this without a timeout, so we just use 5s here.
		struct timespec timeout = { 5, 0 };
		int code = gpiod_line_event_wait (interrupt->line, &timeout);
		if (code < 0)
		{
			perror (STDIO_PREFIX "Failed to wait for line notification");
			continue;
		}
		// Ignore spurrious interrupts (no event actually present).
		if (code != 1)
			continue;

		// Consume the events from the device to indicate they have been handled.
		struct gpiod_line_event event;
		gpiod_line_event_read (interrupt->line, &event);

		// Read the state of the GPIO line to check it is still low (wasn't triggered by noise or something).
		code = gpiod_line_get_value (interrupt->line);
		if (code < 0)
		{
			perror (STDIO_PREFIX "Failed to read line value");
			continue;
		}

		// If the line is low, exit the function.
		if (code == 0)
			return 0;
	}

	#else // Version 2.x.x or later

	const size_t EVENT_BUFFER_CAPACITY = 16;
	struct gpiod_edge_event_buffer* eventBuffer = gpiod_edge_event_buffer_new (EVENT_BUFFER_CAPACITY);
	if (eventBuffer == NULL)
	{
		perror (STDIO_PREFIX "Failed to create GPIO event buffer");
		return errno;
	}

	while (true)
	{
		// Wait indefinitely for the falling edge event.
		int code = gpiod_line_request_wait_edge_events (interrupt->lineRequest, -1);
		if (code < 0)
		{
			perror (STDIO_PREFIX "Failed to wait for line notification");
			continue;
		}
		// Ignore spurrious interrupts (no event actually present).
		if (code != 1)
			continue;

		// Consume the events from the device to indicate they have been handled.
		gpiod_line_request_read_edge_events (interrupt->lineRequest, eventBuffer, EVENT_BUFFER_CAPACITY);

		// Read the state of the GPIO line to check it is still low (wasn't triggered by noise or something).
		code = gpiod_line_request_get_value (interrupt->lineRequest, interrupt->offsets [0]);
		if (code < 0)
		{
			perror (STDIO_PREFIX "Failed to read line value");
			continue;
		}

		// If the line is low, exit the function.
		if (code == 0)
			return 0;
	}

	#endif
}

void shutdownInterruptDealloc (shutdownInterrupt_t* interrupt)
{
	// Version 1.x.x implementation
	#if LIBGPIOD_VERSION_MAJOR == 1

	// Release the GPIO line
	gpiod_line_release (interrupt->line);

	#else // Version 2.x.x or later

	// Release the GPIO line request
	gpiod_line_request_release (interrupt->lineRequest);

	#endif

	// Close the GPIO chip
	gpiod_chip_close (interrupt->chip);

	// Deallocate the structure.
	free (interrupt);
}