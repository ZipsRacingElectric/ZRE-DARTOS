// POSIX
#include <unistd.h>
#include <sys/wait.h>

// C Standard Library
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool terminate = false;

void sigtermHandler (int sig)
{
	(void) sig;

	printf ("Terminating...\n");
	terminate = true;
}

int main (int argc, char** argv)
{
	printf ("Executing '%s'.\n", argv [1]);

	pid_t pid = fork ();
	if (pid == 0)
		execv (argv [1], argv + 1);

	if (signal (SIGTERM, sigtermHandler) == SIG_ERR)
	{
		int code = errno;
		fprintf (stderr, "Warning: Failed to bind SIGTERM handler: %s.\n", strerror (code));
		errno = 0;
	}

	if (signal (SIGINT, sigtermHandler) == SIG_ERR)
	{
		int code = errno;
		fprintf (stderr, "Warning: Failed to bind SIGINT handler: %s.\n", strerror (code));
		errno = 0;
	}

	sleep (1);

	if (waitpid (pid, NULL, WNOHANG | WUNTRACED) != 0)
	{
		fprintf (stderr, "Warning: '%s' terminated early.\n", argv [1]);
	}

	while (!terminate);

	if (kill (pid, SIGTERM) != 0)
	{
		int code = errno;
		fprintf (stderr, "Warning: Failed to kill process: %s.\n", strerror (code));
		errno = 0;
	}

    if (waitpid (pid, NULL, WUNTRACED) == -1)
	{
		int code = errno;
		fprintf (stderr, "Warning failed to wait for program termination: %s.\n", strerror (code));
		errno = 0;
	}


}