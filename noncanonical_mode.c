/*
 * noncanonical_mode.c
 *
 *  Created on: Nov 22, 2010
 *      Author: bogdan
 */

#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static struct termios save_termios;
static int ttysavefd = -1;
static enum{RESET, RAW, CBREAK} ttystate = RESET;

int tty_cbreak(int fd)
{
	int err;
	struct termios buf;

	if (ttystate != RESET)
	{
		errno = EINVAL;
		return -1;
	}
	if (0 > tcgetattr(fd, &buf))
	{
		return -1;
	}
	save_termios = buf;

	buf.c_lflag &= ~(ECHO | ICANON);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;
	if (0 > tcsetattr(fd, TCSAFLUSH, &buf))
	{
		return -1;
	}

	if (0 > tcgetattr(fd, &buf))
	{
		err = errno;
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = err;
		return -1;
	}
	if ((buf.c_lflag & (ECHO | ICANON)) || buf.c_cc[VMIN] != 1 || buf.c_cc[VTIME] != 0)
	{
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = EINVAL;
		return -1;
	}
	ttystate = CBREAK;
	ttysavefd = fd;
	return 0;
}

int tty_raw(int fd)
{
	//int err;
	struct termios buf;

	if (0 > tcgetattr(fd, &buf))
	{
		return -1;
	}
	save_termios = buf;

	buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	buf.c_cflag &= ~(CSIZE | PARENB);
	buf.c_cflag |= CS8;
	buf.c_oflag &= ~OPOST;
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;

	if (0 > tcsetattr(fd, TCSAFLUSH, &buf))
	{
		return -1;
	}

	/*if (0 > tcgetattr(fd, &buf))
	{
		err = errno;
		tcsetattr(fd, TCSAFLUSH, &save_termios);
		errno = err;
		return -1;
	}*/
	//should check if all changes have been made, skip this
	ttystate = RAW;
	ttysavefd = fd;
	return 0;
}

int tty_reset(int fd)
{
	if (ttystate == RESET)
	{
		return 0;
	}
	if (0 > tcsetattr(fd, TCSAFLUSH, &save_termios))
	{
		return -1;
	}
	ttystate = RESET;
	return 0;
}

void tty_atexit(void)
{
	if (0 <= ttysavefd)
	{
		tty_reset(ttysavefd);
	}
}

struct termios* tty_termios(void)
{
	return &save_termios;
}

static void sig_catch(int signo)
{
	printf("signal caught\n");
	tty_reset(STDIN_FILENO);
	exit(0);
}

int main()
{
	int i;
	int c;
	struct sigaction sigact;

	sigact.sa_handler = sig_catch;
	sigact.sa_flags = 0;
	sigemptyset(&sigact.sa_mask);
	if (-1 == sigaction(SIGINT, &sigact, NULL))
	{
		perror("sigaction");
		return -1;
	}
	if (-1 == sigaction(SIGQUIT, &sigact, NULL))
	{
		perror("sigaction");
		return -1;
	}
	if (-1 == sigaction(SIGTERM, &sigact, NULL))
	{
		perror("sigaction");
		return -1;
	}

	if (0 > tty_raw(STDIN_FILENO))
	{
		perror("tty_raw");
		return -1;
	}
	printf("Enter raw mode characters, terminate with DELETE\n");
	while (1 == (i = read(STDIN_FILENO, &c, 1)))
	{
		if ((c &= 255) == 0177)
		{
			break;
		}
		printf("%o\n", c);
	}
	if (0 > tty_reset(STDIN_FILENO))
	{
		perror("tty_reset");
		return -1;
	}
	if (i <= 0)
	{
		fprintf(stderr, "read error\n");
		return -1;
	}
	if (0 > tty_cbreak(STDIN_FILENO))
	{
		perror("tty_cbreak");
		return -1;
	}
	printf("\nEnter cbreak mode characters, terminate with SIGINT\n");
	while (1 == (i = read(STDIN_FILENO, &c, 1)))
	{
		c &= 255;
		printf("%o\n", c);
	}
	if (0 > tty_reset(STDIN_FILENO))
	{
		perror("tty_reset");
		return -1;
	}
	if (0 >= i)
	{
		fprintf(stderr, "read error\n");
	}
	exit(0);
}
