/*
 * print_window_size.c
 *
 *  Created on: Nov 23, 2010
 *      Author: bogdan
 */

#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void print_size(int fd)
{
	struct winsize size;

	if (0 > ioctl(fd, TIOCGWINSZ, (char*)&size))
	{
		perror("ioctl");
		return;
	}
	printf("%d rows, %d columns\n", size.ws_row, size.ws_col);
}

void sig_winch(int signo)
{
	printf("SIGWINCH received\n");
	print_size(STDIN_FILENO);
}

int main()
{
	struct sigaction sigact;

	if (0 == isatty(STDIN_FILENO))
	{
		return EXIT_FAILURE;
	}


	sigact.sa_handler = sig_winch;
	sigact.sa_flags = 0;
	if (-1 == sigemptyset(&sigact.sa_mask))
	{
		perror("sigemtyset");
		return EXIT_FAILURE;
	}
	if (-1 == sigaction(SIGWINCH, &sigact, NULL))
	{
		perror("sigaction");
		return EXIT_FAILURE;
	}
	print_size(STDIN_FILENO);
	while(1)
	{
		pause();
	}
	return EXIT_SUCCESS;
}
