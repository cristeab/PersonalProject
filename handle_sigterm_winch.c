/*
 * handle_sigterm_winch.c
 *
 *  Created on: Nov 24, 2010
 *      Author: bogdan
 */

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/syslog.h>

void sigterm_hnd(int signo)
{
	printf("SIGTERM received\n");
}

void sigwinch_hnd(int signo)
{
	struct winsize wsize;

	printf("SIGWINCH received\n");
	if (0 > ioctl(STDIN_FILENO, TIOCGWINSZ, (char*)&wsize))
	{
		perror("ioctl");
		return;
	}
	printf("Column: %d\n", wsize.ws_col);
	printf("Row: %d\n", wsize.ws_row);
}

int main()
{
	struct sigaction sigact;
	sigset_t sigset_empty;

	sigact.sa_flags = 0;
	if (-1 == sigemptyset(&sigset_empty))
	{
		perror("sigemptyset");
		return EXIT_FAILURE;
	}
	sigact.sa_mask = sigset_empty;
	sigact.sa_handler = sigterm_hnd;
	if (-1 == sigaction(SIGTERM, &sigact, NULL))
	{
		perror("sigaction");
		return EXIT_FAILURE;
	}
	sigact.sa_handler = sigwinch_hnd;
	if (-1 == sigaction(SIGWINCH, &sigact, NULL))
	{
		perror("sigaction");
		return EXIT_FAILURE;
	}

	syslog(LOG_INFO, "waiting for signals");
	while (1)
	{
		sigsuspend(&sigset_empty);
	}

	return EXIT_SUCCESS;
}
