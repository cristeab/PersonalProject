/*
 * rt_timers.c
 *
 *  Created on: Dec 8, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>

#define MAX_EXPIRE 2
volatile int expire;

void timer_handler(int signo, siginfo_t *info, void *context)
{
	int overrun = 0;
	printf("signal details: signal %d, code %d\n", info->si_signo,
			info->si_code);
	if (SI_TIMER == info->si_code)
	{
		printf("timer id %d\n", info->si_timerid);
		++expire;
		if (-1 != (overrun = timer_getoverrun((timer_t)&info->si_timerid)) && 0
				!= overrun)
		{
			printf("timer overrun %d\n", overrun);
			expire += overrun;
		}
	}
}

START_TEST (test_template)
{
	struct timespec ts, tm, sleep;
	sigset_t mask;
	struct sigevent sigev;
	struct sigaction sa;
	struct itimerspec ival;
	timer_t tid;

	if (0 != clock_getres(CLOCK_MONOTONIC, &ts))
	{
		perror("clock_getres");
		fail();
	}
	if (0 != clock_gettime(CLOCK_MONOTONIC, &tm))
	{
		perror("clock_gettime");
		fail();
	}
	printf("CLOCK_MONOTONIC res: %ld s %ld ns\n", ts.tv_sec, ts.tv_nsec);
	printf("system uptime %ld s %ld ns\n", tm.tv_sec, tm.tv_nsec);

	//setup a signal handler at timer expiration
	if (0 != sigemptyset(&mask))
	{
		perror("sigemptyset");
		fail();
	}
	if (0 != sigprocmask(SIG_SETMASK, &mask, NULL))
	{
		perror("sigprocmask");
		fail();
	}
	//register handler
	sa.sa_flags = SA_SIGINFO;
	if (0 != sigemptyset(&sa.sa_mask))
	{
		perror("sigemptyset");
		fail();
	}
	sa.sa_sigaction = timer_handler;
	if (0 != sigaction(SIGRTMIN, &sa, NULL))
	{
		perror("sigaction");
		fail();
	}
	//create the timer
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGRTMIN;
	sigev.sigev_value.sival_int = 1;
	if (0 != timer_create(CLOCK_MONOTONIC, &sigev, &tid))
	{
		perror("timer_create");
		fail();
	}
	printf("timer id %p\n", tid);
	//arm the timer (expiration after four seconds)
	ival.it_value.tv_sec = 5;
	ival.it_value.tv_nsec = 0;
	ival.it_interval.tv_sec = 4;
	ival.it_interval.tv_nsec = 0;
	if (0 != timer_settime(tid, 0, &ival, NULL))
	{
		perror("timer_settime");
		fail();
	}
	//sleep and wait for signal
	while(1)
	{
		sleep.tv_sec = 3;
		sleep.tv_nsec = 0;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &sleep, NULL);
		printf("woken up\n");
		if (MAX_EXPIRE <= expire)
		{
			printf("Programming quitting\n");
			//disarm the timer
			memset(&ival, 0, sizeof(ival));
			if (0 != timer_settime(tid, 0, &ival, NULL))
			{
				perror("timer_settime");
				fail();
			}
			exit(EXIT_SUCCESS);
		}
	}
}
END_TEST

Suite* test_suite(void)
{
        Suite *s = suite_create("templated_suite");

        /* Core test case */
        TCase *tc_core = tcase_create("Core");
        tcase_set_timeout(tc_core, 10);
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
