// DARTOS Init-System ---------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date: Created: 2026.01.20
//
// Description: TODO(Barach)

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "shutdown_interrupt.h"

// POSIX
#include <unistd.h>
#include <sys/wait.h>

// C Standard Library
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Constants ------------------------------------------------------------------------------------------------------------------

#define STDIO_PREFIX "[DARTOS INIT-SYSTEM] "

// Entrypoints ----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	if (argc < 2)
		return -1;

	char* initProcess = argv [1];
	pid_t initPid = fork ();
	if (initPid == 0)
	{
		char* argv [] = { initProcess, NULL };
		execvp (initProcess, argv);
		int code = errno;
		fprintf (stderr, STDIO_PREFIX "Failed to execute process '%s': %s.\n", initProcess, strerror (code));
		return errno;
	}

	waitpid (initPid, NULL, 0);

	size_t processCount = argc - 2;
	char** processPathes = argv + 2;

	shutdownInterrupt_t interrupt;

	if (shutdownInterruptInit (&interrupt, "init-system", "gpiochip0", 21) != 0)
		return errno;

	pid_t* pids = malloc (sizeof (pid_t) * processCount);
	if (pids == NULL)
	{
		perror ("Failed to allocate process PIDs");
		return -1;
	}

	for (size_t index = 0; index < processCount; ++index)
	{
		printf (STDIO_PREFIX "Executing process '%s'.\n", processPathes [index]);
		pids [index] = fork ();
		if (pids [index] == 0)
		{
			char* argv [] = { processPathes [index], NULL };
			execvp (processPathes [index], argv);
			int code = errno;
			fprintf (stderr, STDIO_PREFIX "Failed to execute process '%s': %s.\n", processPathes [index], strerror (code));
			free (pids);
			return errno;
		}
	}

	nanosleep (&(struct timespec) { .tv_nsec = 10000000 }, NULL);

	for (size_t index = 0; index < processCount; ++index)
	{
		if (waitpid (pids [index], NULL, WNOHANG) != 0)
		{
			fprintf (stderr, STDIO_PREFIX "Warning: Process '%s' terminated early.\n", processPathes [index]);
			pids [index] = 0;
		}
	}

	shutdownInterruptPoll (&interrupt);

	printf (STDIO_PREFIX "Terminating...\n");

	struct timespec timeStart;
	clock_gettime (CLOCK_MONOTONIC, &timeStart);

	for (size_t index = 0; index < processCount; ++index)
	{
		if (pids [index] != 0)
		{
			if (kill (pids [index], SIGTERM) != 0)
			{
				int code = errno;
				fprintf (stderr, STDIO_PREFIX "Failed to kill process '%s': %s.\n", processPathes [index], strerror (code));
				errno = 0;
			}
		}
	}

	// Block until all child processes have terminated.
	while (!(wait (NULL) == -1 && errno == ECHILD));

	free (pids);

	struct timespec timeEnd;
	clock_gettime (CLOCK_MONOTONIC, &timeEnd);

	float timeDiff = (timeEnd.tv_sec - timeStart.tv_sec) * 1e3f + (timeEnd.tv_nsec - timeStart.tv_nsec) * 1e-6f;

	printf (STDIO_PREFIX "All processes terminated in %f ms.\n", timeDiff);
	
	shutdownInterruptDealloc (&interrupt);

	return 0;
}