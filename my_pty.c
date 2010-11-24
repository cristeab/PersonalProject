/*
 * my_pty.c
 *
 *  Created on: Nov 24, 2010
 *      Author: bogdan
 */

#include <termios.h>
#define __USE_XOPEN
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

int ptym_open(char *pts_name, int pts_namesz)
{
	char *ptr = NULL;
	int fdm;

	strncpy(pts_name, "/dev/ptyXX", pts_namesz);
	pts_name[pts_namesz-1] = '\0';
	if (0 > (fdm = posix_openpt(O_RDWR)))
	{
		return -1;
	}
	if (0 > grantpt(fdm))
	{
		close (fdm);
		return -2;
	}
	if (0 > unlockpt(fdm))
	{
		close(fdm);
		return -3;
	}
	if (NULL == (ptr = ptsname(fdm)))
	{
		close(fdm);
		return -4;
	}

	strncpy(pts_name, ptr, pts_namesz);
	pts_name[pts_namesz-1] = '\0';
	return fdm;
}

int ptys_open(char *pts_name)
{
	int fds;

	if (0 > (fds = open(pts_name, O_RDWR)))
	{
		return -5;
	}
	return fds;
}

pid_t pty_fork(int *ptrfdm, char *slave_name, int slave_namesz,
		const struct termios *slave_termios, const struct winsize *slave_winsize)
{
	int fdm, fds;
	pid_t pid;
	char pts_name[20];

	if (0 > (fdm = ptym_open(pts_name, sizeof(pts_name))))
	{
		fprintf(stderr, "ptym_open");
		return -1;
	}
	if (NULL != slave_name)
	{
		strncpy(slave_name, pts_name, slave_namesz);
		slave_name[slave_namesz-1] = '\0';
	}

	if (0 > (pid = fork()))
	{
		return -1;
	} else if (pid == 0)
	{
		if (0 > setsid())
		{
			perror("setsid\n");
			return -1;
		}
		if (0 > (fds = ptys_open(pts_name)))
		{
			fprintf(stderr, "pts_open\n");;
			return -1;
		}
		close(fdm);
		if (NULL != slave_termios)
		{
			if (0 > tcsetattr(fds, TCSANOW, slave_termios))
			{
				perror("tcsetattr");
				return -1;
			}
		}
		if (NULL != slave_winsize)
		{
			if (0 > ioctl(fds, TIOCSWINSZ, slave_winsize))
			{
				perror("ioctl");
				return -1;
			}
		}
		if (STDIN_FILENO != dup2(fds, STDIN_FILENO))
		{
			perror("dup2 - stdin");
			return -1;
		}
		if (STDOUT_FILENO != dup2(fds, STDOUT_FILENO))
		{
			perror("dup2 - stdout");
			return -1;
		}
		if (STDERR_FILENO != dup2(fds, STDERR_FILENO))
		{
			perror("dup2 - stderr");
			return -1;
		}
		return 0;
	} else {
		*ptrfdm = fdm;
		return pid;
	}
}

#define OPTSTR "+d:einv"

static void set_noecho(int);
//void do_driver(char*);
void loop(int, int);
void loop_one_process(int, int);

int main(int argc, char *argv[])
{
	int fdm, c, ignoreeof, interactive, noecho, verbose;
	pid_t pid;
	char *driver;
	char slave_name[20];
	struct termios orig_termios;
	struct winsize size;

	interactive = isatty(STDIN_FILENO);
	ignoreeof = 0;
	noecho = 0;
	verbose = 0;
	driver = NULL;

	opterr = 0;
	while (EOF != (c = getopt(argc, argv, OPTSTR)))
	{
		switch(c)
		{
		case 'd':
			driver = optarg;
			break;
		case 'e':
			noecho = 1;
			break;
		case 'i':
			ignoreeof = 1;
			break;
		case 'n':
			interactive = 0;
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
			fprintf(stderr, "unrecognized option -%c", optopt);
			return EXIT_FAILURE;
		}
	}
	if (optind >= argc)
	{
		fprintf(stderr, "usage: tpy [-d driver -einv] program [arg ...]");
		return EXIT_FAILURE;
	}

	if (interactive)
	{
		if (0 > tcgetattr(STDIN_FILENO, &orig_termios))
		{
			perror("tcgetattr");
			return EXIT_FAILURE;
		}
		if (0 > ioctl(STDIN_FILENO, TIOCGWINSZ, (char*)&size))
		{
			perror("ioctl");
			return EXIT_FAILURE;
		}
		pid = pty_fork(&fdm, slave_name, sizeof(slave_name), &orig_termios, &size);
	} else {
		pid = pty_fork(&fdm, slave_name, sizeof(slave_name), NULL, NULL);
	}

	if (0 > pid)
	{
		fprintf(stderr, "fork error\n");
		return EXIT_FAILURE;
	} else if (0 == pid)
	{
		if (noecho)
		{
			set_noecho(STDIN_FILENO);
		}
		if (0 > execvp(argv[optind], &argv[optind]))
		{
			perror("execvp");
			return EXIT_FAILURE;
		}
	}

	if (verbose)
	{
		fprintf(stderr, "slave name = %s\n", slave_name);
		if (NULL != driver)
		{
			fprintf(stderr, "driver = %s\n", driver);
		}
	}

	if (interactive && (NULL ==driver))
	{
		/*if (0 > tty_raw(STDIN_FILENO))
		{
			fprintf("tty_raw\n");
			return EXIT_FAILURE;
		}
		if (0 > atexit(tty_exit))
		{
			perror("atexit");
			return EXIT_FAILURE;
		}*/
	}

	if (driver)
	{
		//do_driver(driver);
	}

	//loop(fdm, ignoreeof);
	loop_one_process(fdm, ignoreeof);

	return EXIT_SUCCESS;
}

static void set_noecho(int fd)
{
	struct termios stermios;

	if (0 > tcgetattr(fd, &stermios))
	{
		perror("tcgetattr");
		return;
	}
	stermios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	stermios.c_oflag &= ~ONLCR;

	if (0 > tcsetattr(fd, TCSANOW, &stermios))
	{
		perror("tcsetattr");
	}
}

#define BUFFSIZE 512
static volatile sig_atomic_t sigcaught;

static void sig_term(int signo)
{
	sigcaught = 1;
}

void loop(int ptym, int ignoreeof)
{
	pid_t child;
	int nread;
	char buf[BUFFSIZE];
	struct sigaction sigact;

	if (0 > (child = fork()))
	{
		perror("fork");
		return;
	} else if (0 == child)
	{
		//child
		while (1)
		{
			if (0 > (nread = read(STDIN_FILENO, buf, BUFFSIZE)))
			{
				perror("read");
				return;
			} else if (0 == nread)
			{
				break;
			}
			if (nread != write(ptym, buf,nread))
			{
				perror("write");
				return;
			}
		}

		if (0 == ignoreeof)
		{
			kill(getppid(), SIGTERM);
		}
		exit(EXIT_SUCCESS);
	}

	//parent
	sigact.sa_handler = sig_term;
	sigact.sa_flags = 0;
	if (0 > sigemptyset(&sigact.sa_mask))
	{
		perror("sigemptyset");
		return;
	}
	if (0 > sigaction(SIGTERM, &sigact, NULL))
	{
		perror("sigaction");
		return;
	}

	while (1)
	{
		if (0 >= (nread = read(ptym, buf, BUFFSIZE)))
		{
			break;
		}
		if (nread != write(STDOUT_FILENO, buf, nread))
		{
			perror("write");
			return;
		}
	}

	if (0 == sigcaught)
	{
		kill(child, SIGTERM);
	}
}

void loop_one_process(int ptym, int ignoreeof)
{
	int nread;
	char buf[BUFFSIZE];
	fd_set readfds;
	fd_set readfds_current;

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	FD_SET(ptym, &readfds);

	while (1)
	{
		readfds_current = readfds;
		if (0 > select(ptym + 1, &readfds_current, NULL, NULL, NULL))
		{
			perror("select");
			break;
		}
		if (FD_ISSET(STDIN_FILENO, &readfds_current))
		{
			if (0 > (nread = read(STDIN_FILENO, buf, BUFFSIZE)))
			{
				perror("read");
				break;
			}
			else if (0 == nread)
			{
				break;
			}
			if (nread != write(ptym, buf,nread))
			{
				perror("write");
				break;
			}
		}
		if (FD_ISSET(ptym, &readfds_current))
		{
			if (0 >= (nread = read(ptym, buf, BUFFSIZE)))
			{
				break;
			}
			if (nread != write(STDOUT_FILENO, buf, nread))
			{
				perror("write");
				break;
			}
		}
	}
	exit(EXIT_FAILURE);
}
