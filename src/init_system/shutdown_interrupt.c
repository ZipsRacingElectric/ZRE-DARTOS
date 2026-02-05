// Header
#include "shutdown_interrupt.h"

int shutdownInterruptInit (shutdownInterrupt_t* interrupt, const char* consumer, const char* chipName, unsigned int lineNumber)
{
	interrupt->chip = gpiod_chip_open_by_name (chipName);
	if (interrupt->chip == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open GPIO chip '%s': %s.\n", chipName, strerror (code));
		return code;
	}

	interrupt->line = gpiod_chip_get_line(interrupt->chip, lineNumber);
	if (interrupt->line == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to get GPIO line %u: %s.\n", lineNumber, strerror (code));
		gpiod_chip_close (interrupt->chip);
		return code;
	}

	int code = gpiod_line_request_falling_edge_events_flags(interrupt->line, consumer, GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);
	if (code < 0)
	{
		perror ("Failed to request GPIO line for falling edge notification");
		gpiod_line_release (interrupt->line);
		gpiod_chip_close (interrupt->chip);
		return code;
	}

	return 0;
}

int shutdownInterruptPoll (shutdownInterrupt_t* interrupt)
{
	while (true)
	{
		// TODO(Barach): Max timeout?
		struct timespec timeout = { 5, 0 };
		int code = gpiod_line_event_wait (interrupt->line, &timeout);
		if (code < 0)
		{
			perror ("Failed to wait for line notification");
			continue;
		}
		if (code != 1)
			continue;

		struct gpiod_line_event event;
		gpiod_line_event_read (interrupt->line, &event);

		code = gpiod_line_get_value (interrupt->line);
		if (code < 0)
		{
			perror ("Failed to read line value");
			continue;
		}

		if (code == 0)
			return 0;
	}
}

void shutdownInterruptDealloc (shutdownInterrupt_t* interrupt)
{
	gpiod_line_release (interrupt->line);
	gpiod_chip_close (interrupt->chip);
}