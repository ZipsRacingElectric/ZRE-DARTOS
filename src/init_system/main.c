// DART-OS Init-System --------------------------------------------------------------------------------------------------------
//
// Author: Cole Barach
// Date: Created: 2026.01.20
//
// Description: See doc/init_system_application.md for more details.

// Includes -------------------------------------------------------------------------------------------------------------------

// Includes
#include "init_system_stdio.h"
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
#include <limits.h>

// Functions ------------------------------------------------------------------------------------------------------------------

/**
 * @brief Converts a GPIO line string into a GPIO line number.
 * @param str The string to convert.
 * @return The line number, if successful, -1 otherwise.
 */
int getGpioLine (char* str)
{
	char* end;
	unsigned long line = strtoul (str, &end, 0);
	if (end == str)
		return -1;

	if (line > UINT_MAX)
		return -1;

	return line;
}

/**
 * @brief Executes and waits for the termination of a pre-execution / post-execution application.
 * @param application The application to run.
 * @return 0 if successful, the error code otherwise.
 */
int execPrePostExecApplication (char* application)
{
	printf (STDIO_PREFIX "Executing '%s'...\n", application);

	// Executure the pre-exec application
	pid_t initPid = fork ();
	if (initPid == 0)
	{
		char* argv [] = { application, NULL };
		execvp (application, argv);

		// execvp only returns on failure.
		int code = errno;
		fprintf (stderr, STDIO_PREFIX "Failed to execute process '%s': %s.\n", application, strerror (code));
		return errno;
	}

	waitpid (initPid, NULL, 0);

	return 0;
}

/**
 * @brief Executes an array of applications. Note this does not wait for application termination.
 * @param applicationPathes The array containing the path of each application to execute.
 * @param applicationPids The array to write to PID of each application into.
 * @param applicationCount The size of @c applicationPathes and @c applicationPids .
 */
void execApplications (char** applicationPathes, pid_t* applicationPids, size_t applicationCount)
{
	for (size_t index = 0; index < applicationCount; ++index)
	{
		printf (STDIO_PREFIX "Executing application '%s'.\n", applicationPathes [index]);
		applicationPids [index] = fork ();
		if (applicationPids [index] == 0)
		{
			char* argv [] = { applicationPathes [index], NULL };
			execvp (applicationPathes [index], argv);
			int code = errno;

			// execvp only returns on failure.
			// - Note: This is in the context of the child application, not the init-system itself, so we cannot allow the
			//   process to keep running.
			fprintf (stderr, STDIO_PREFIX "Failed to execute process '%s': %s.\n", applicationPathes [index], strerror (code));
			free (applicationPids);
			exit (errno);
		}
	}
}

/**
 * @brief Checks the status of an array of applications, printing a warning on an early termination.
 * @param applicationPathes The array containing the path of each application to check.
 * @param applicationPids The array containing the PID of each application to check.
 * @param applicationCount The size of @c applicationPathes and @c applicationPids .
 */
void checkApplications (char** applicationPathes, pid_t* applicationPids, size_t applicationCount)
{
	// Check if each individual application has exited
	for (size_t index = 0; index < applicationCount; ++index)
	{
		if (waitpid (applicationPids [index], NULL, WNOHANG) != 0)
		{
			fprintf (stderr, STDIO_PREFIX "Warning: Process '%s' terminated early.\n", applicationPathes [index]);
			applicationPids [index] = 0;
		}
	}
}

/**
 * @brief Terminates an array of applications.
 * @param applicationPathes The array containing the path of each application to terminate.
 * @param applicationPids The array containing the PID of each application to terminate.
 * @param applicationCount The size of @c applicationPathes and @c applicationPids .
 */
void terminateApplications (char** applicationPathes, pid_t* applicationPids, size_t applicationCount, struct timespec* timeStart)
{
	// Send the termination signal to each application
	for (size_t index = 0; index < applicationCount; ++index)
	{
		if (applicationPids [index] != 0)
		{
			if (kill (applicationPids [index], SIGTERM) != 0)
			{
				int code = errno;
				fprintf (stderr, STDIO_PREFIX "Failed to terminate process '%s': %s.\n", applicationPathes [index], strerror (code));
				errno = 0;
			}
		}
	}

	// Block until all child applications have terminated.
	int pid;
	do
	{
		// Wait for a child application to exit.
		pid = wait (NULL);
		if (pid == -1 && errno == ECHILD)
			break;

		// Print the application that exited and its time.
		struct timespec timeCurrent;
		clock_gettime (CLOCK_MONOTONIC, &timeCurrent);
		float timeDiff = (timeCurrent.tv_sec - timeStart->tv_sec) * 1e3f + (timeCurrent.tv_nsec - timeStart->tv_nsec) * 1e-6f;

		for (size_t index = 0; index < applicationCount; ++index)
			if (applicationPids [index] == pid)
				printf ("Application '%s' terminated in %f ms.\n", applicationPathes [index], timeDiff);
	} while (true);
}

// Entrypoints ----------------------------------------------------------------------------------------------------------------

int main (int argc, char** argv)
{
	// Validate the application usage.
	if (argc < 5)
	{
		fprintf (stderr, "Invalid usage. Usage: init-system <GPIO Chip> <GPIO Line> <Pre-Execution Application> "
			"<Post-Execution Application> <Application 0> <Application 1> ...\n");
		return -1;
	}

	// Get the GPIO chip and GPIO line from standard arguments.
	char* gpioChip = argv [1];
	char* gpioLineStr = argv [2];
	int gpioLine = getGpioLine (gpioLineStr);
	if (gpioLine < 0)
	{
		fprintf (stderr, STDIO_PREFIX "Invalid GPIO line '%s'.\n", gpioLineStr);
		return EINVAL;
	}

	// Initialize the shutdown interrupt GPIO. When the device is powering down, this will trigger the init-system to terminate
	// all child applications.
	shutdownInterrupt_t* interrupt = shutdownInterruptInit ("init-system", gpioChip, gpioLine);
	if (interrupt == NULL)
		return errno;

	// Execute the pre-exec application.
	if (execPrePostExecApplication (argv [3]) != 0)
		return errno;

	// Get the number of applications to execute and their pathes
	size_t applicationCount = argc - 5;
	char** applicationPathes = argv + 5;

	// Allocate an array for storing the application PIDs.
	pid_t* applicationPids = malloc (sizeof (pid_t) * applicationCount);
	if (applicationPids == NULL)
	{
		perror (STDIO_PREFIX "Failed to allocate application PIDs");
		return errno;
	}

	// Execute the applications, wait briefly, then check for any early terminations.
	execApplications (applicationPathes, applicationPids, applicationCount);
	nanosleep (&(struct timespec) { .tv_nsec = 10000000 }, NULL);
	checkApplications (applicationPathes, applicationPids, applicationCount);

	// Wait for the shutdown interrupt to indicate the device is shutting down.
	if (shutdownInterruptPoll (interrupt) != 0)
	{
		perror (STDIO_PREFIX "Failed to poll shutdown interrupt");
		return errno;
	}
	printf (STDIO_PREFIX "Terminating...\n");

	// Time the shutdown sequence
	struct timespec timeStart;
	clock_gettime (CLOCK_MONOTONIC, &timeStart);

	// Terminate all the remaining applications
	terminateApplications (applicationPathes, applicationPids, applicationCount, &timeStart);
	free (applicationPids);

	// Release the shutdown GPIO
	shutdownInterruptDealloc (interrupt);

	// Execute the post-exec application.
	if (execPrePostExecApplication (argv [4]) != 0)
		return errno;

	// Finish timing and print to stdout
	struct timespec timeEnd;
	clock_gettime (CLOCK_MONOTONIC, &timeEnd);
	float timeDiff = (timeEnd.tv_sec - timeStart.tv_sec) * 1e3f + (timeEnd.tv_nsec - timeStart.tv_nsec) * 1e-6f;
	printf (STDIO_PREFIX "All processes terminated in %f ms.\n", timeDiff);

	return 0;
}