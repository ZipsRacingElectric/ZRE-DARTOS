#ifndef SHUTDOWN_INTERRUPT_H
#define SHUTDOWN_INTERRUPT_H

// LibGPIOD
#include <gpiod.h>

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct
{
	struct gpiod_chip* chip;
	struct gpiod_line* line;
} shutdownInterrupt_t;

int shutdownInterruptInit (shutdownInterrupt_t* interrupt, const char* consumer, const char* chipName, unsigned int lineNumber);

int shutdownInterruptPoll (shutdownInterrupt_t* interrupt);

void shutdownInterruptDealloc (shutdownInterrupt_t* interrupt);

#endif // SHUTDOWN_INTERRUPT_H