/*
 * rt_signals.c
 *
 *  Created on: Dec 8, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

int parent(pid_t cpid)
{
	union sigval value;
	sleep(1);
	value.sival_int = 1;
	if (0 != sigqueue(cpid, SIGRTMIN, value))
	{
		perror("sigqueue");
		fail();
	}
	sleep(1);
	value.sival_int = 2;
	if (0 != sigqueue(cpid, SIGRTMIN+1, value))
	{
		perror("sigqueue");
		fail();
	}
	sleep(1);
	value.sival_int = 3;
	if (0 != sigqueue(cpid, SIGRTMIN, value))
	{
		perror("sigqueue");
		fail();
	}
	return EXIT_SUCCESS;
}

void rt_handler(int signum, siginfo_t *siginfo, void *extra)
{
	printf("signal %d received code %d, value %d\n", siginfo->si_signo, siginfo->si_code, siginfo->si_int);
}

int child()
{
	sigset_t mask, oldmask;
	siginfo_t siginfo;
	struct sigaction action;
	struct timespec tv;
	int count = 0, recv_sig;

	if (0 != sigemptyset(&mask))
	{
		perror("sigemptyset");
		fail();
	}
	if (0 != sigaddset(&mask, SIGRTMIN))
	{
		perror("sigaddset");
		fail();
	}
	if (0 != sigaddset(&mask, SIGRTMIN+1))
	{
		perror("sigaddset");
		fail();
	}
	if (0 != sigprocmask(SIG_BLOCK, &mask, &oldmask))
	{
		perror("sigprocmask");
		fail();
	}
	tv.tv_sec = 1;
	tv.tv_nsec = 0;
	while (count < 2)
	{
		if (-1 == (recv_sig = sigtimedwait(&mask, &siginfo, &tv)))
		{
			if (EAGAIN == errno)
			{
				printf("timed out\n");
				continue;
			} else
			{
				perror("sigtimedwait");
				fail();
			}
		} else
		{
			printf("received signal %d\n", recv_sig);
			++count;
		}
	}

	action.sa_flags = SA_SIGINFO;
	if (0 != sigemptyset(&action.sa_mask))
	{
		perror("sigemptyset");
		fail();
	}
	action.sa_sigaction = rt_handler;
	if (0 != sigaction(SIGRTMIN, &action, NULL))
	{
		perror("sigaction");
		fail();
	}

	sigsuspend(&oldmask);

	return EXIT_SUCCESS;
}

START_TEST (test_template)
{
	pid_t pid;

	if (0 > (pid = fork()))
	{
		perror("fork");
		fail();
	} else if (0 == pid)
	{
		child();
	} else
	{
		parent(pid);
	}
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
