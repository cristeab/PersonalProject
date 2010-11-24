/*
 * pseudo_terminal.c
 *
 *  Created on: Nov 24, 2010
 *      Author: bogdan
 */

#include <check.h>
#define __USE_XOPEN
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/syslog.h>

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

#define BUF_LEN 128
START_TEST (test_template)
{
	char buf[BUF_LEN];
	pid_t pid;
	int fdm = -1;

	//open master pts
	fail_unless(ptym_open(buf, BUF_LEN)>0);
	fail_unless(strncmp(buf, "/dev/pts", 8) == 0);

	//open slave pts
	fail_unless(ptys_open(buf) > 0);

	//for a new process with a pts as controlling terminal
	pid = pty_fork(&fdm, buf, BUF_LEN, NULL, NULL);
	fail_if(pid < 0);
	if (0 == pid)
	{
		//syslog(LOG_INFO, "child");
		fail_unless(0 == isatty(fdm));
		fail_unless(EBADF == errno);
	} else
	{
		//syslog(LOG_INFO, "parent");
		fail_unless(1 == isatty(fdm));
	}
	check_waitpid_and_exit(pid);
}
END_TEST

Suite* test_suite(void)
{
        Suite *s = suite_create("templated_suite");

        /* Core test case */
        TCase *tc_core = tcase_create("Core");
        tcase_add_test(tc_core, test_template);
        suite_add_tcase(s, tc_core);

        return s;
}

int main()
{
        int number_failed;
        Suite *s = test_suite();
        SRunner *sr = srunner_create(s);
        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
