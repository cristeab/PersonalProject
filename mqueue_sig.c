/*
 * mqueue.c
 *
 *  Created on: Dec 7, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/syslog.h>

#define QUEUE_NAME "/my_queue"
#define PRIORITY 1
#define SIZE 256

void sig_hnd(int signo)
{
	syslog(LOG_INFO, "signal hdn called");
}

START_TEST (test_template)
{
	pid_t pid;
	mqd_t ds;
	const char text[] = "Hello RT POSIX World";
	char recv_text[SIZE];
	struct mq_attr queue_attr, queue_attr_old;
	unsigned int prio;
	sigset_t sigset, empty_sigset;
	struct sigevent not;
	struct sigaction sigact;

	queue_attr.mq_maxmsg = 32;
	queue_attr.mq_msgsize = SIZE;
	queue_attr.mq_flags = 0;
	queue_attr.mq_curmsgs = 0;
	memset(&queue_attr_old, 0, sizeof(queue_attr_old));

	//open queue for read/write
	mq_unlink(QUEUE_NAME);//make sure that this queue doesn't exist
	if ((mqd_t)-1 == (ds = mq_open(QUEUE_NAME, O_CREAT | O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR, &queue_attr)))
	{
		perror("mq_open");
		fail();
	}

	memset(&queue_attr, 0, sizeof(queue_attr));
	if ((mqd_t)-1 == mq_getattr(ds, &queue_attr))
	{
		perror("mq_getattr");
		fail();
	}
	if (O_NONBLOCK != (queue_attr.mq_flags & O_NONBLOCK))
	{
		fprintf(stderr, "nonblock flag is not set: %ld\n", queue_attr.mq_flags);
		fprintf(stderr, "maxsmg = %ld\n", queue_attr.mq_maxmsg);
		fprintf(stderr, "maxsize = %ld\n", queue_attr.mq_msgsize);
		fail();
	}

	if (-1 == sigemptyset(&sigset))
	{
		perror("sigemptyset");
		fail();
	}
	empty_sigset = sigset;
	if (-1 == sigaddset(&sigset, SIGUSR1))
	{
		perror("sigaddset");
		fail();
	}
	//block signal
	if (-1 == sigprocmask(SIG_BLOCK, &sigset, NULL))
	{
		perror("sigprocsignal");
		fail();
	}
	//register signal handler
	sigact.sa_flags = 0;
	sigact.sa_mask = empty_sigset;
	sigact.sa_handler = sig_hnd;
	if (-1 == sigaction(SIGUSR1, &sigact, NULL))
	{
		perror("sigaction");
		fail();
	}

	//register notifications
	not.sigev_notify = SIGEV_SIGNAL;
	not.sigev_signo = SIGUSR1;
	if ((mqd_t)-1 == mq_notify(ds, &not))
	{
		perror("mq_notify");
		fail();
	}

	syslog(LOG_INFO, "before creating the child");

	if (0 > (pid = fork()))
	{
		perror("fork");
		fail();
	} else if (0 == pid)
	{
		//the child sends a message through the RT message queue
		if (-1 == mq_send(ds, text, strlen(text), PRIORITY))
		{
			perror("mq_send");
			fail();
		}
		if (-1 == mq_close(ds))
		{
			perror("mq_close");
			fail();
		}
		syslog(LOG_INFO, "message was send");
		exit(EXIT_SUCCESS);
	}
	//parent receives the message: a signal is delivered when a message is to be received
	//make sure that the receive is non blocking
	if ((mqd_t)-1 == mq_getattr(ds, &queue_attr_old))
	{
		perror("mq_getattr");
		fail();
	}
	if (!(queue_attr_old.mq_flags & O_NONBLOCK))
	{
		fprintf(stderr, "The reception is blocking. Setting flags to unblocking receive\n");
		queue_attr_old.mq_flags = O_NONBLOCK;
		if ((mqd_t)-1 == mq_setattr(ds, &queue_attr_old, NULL))
		{
			perror("mq_setattr");
			fail();
		}
	}
	//wait for signal delivery
	syslog(LOG_INFO, "waiting for signal delivery");
	sigsuspend(&empty_sigset);
	//receive the message (no wait)
	if (-1 == mq_receive(ds, recv_text, SIZE, &prio))
	{
		perror("mq_receive");
		fail();
	}
	fail_unless(0 == strncmp(text, recv_text, strlen(text)));

	if (-1 == mq_close(ds))
	{
		perror("mq_close");
		fail();
	}
	if (-1 == mq_unlink(QUEUE_NAME))
	{
		perror("mq_unlink");
		fail();
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
