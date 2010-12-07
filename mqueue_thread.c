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

const char text[] = "Hello RT POSIX World";
char msg_recved = 0;

void thread_starter(union sigval siginfo)
{
	unsigned int prio;
	char recv_text[SIZE];

	//receive the message (no wait)
	if (-1 == mq_receive(siginfo.sival_int, recv_text, SIZE, &prio))
	{
		perror("mq_receive");
		fail();
	}

	fail_unless(0 == strncmp(text, recv_text, strlen(text)));
	msg_recved = 1;
}

START_TEST (test_template)
{
	pid_t pid;
	mqd_t ds;
	struct mq_attr queue_attr, queue_attr_old;
	struct sigevent not;

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

	//register notifications
	memset(&not, 0, sizeof(struct sigevent));
	not.sigev_notify = SIGEV_THREAD;
	not.sigev_notify_function = thread_starter;
	not.sigev_notify_attributes = NULL;
	not.sigev_value.sival_int = ds;
	if ((mqd_t)-1 == mq_notify(ds, &not))
	{
		perror("mq_notify");
		fail();
	}

	if (0 > (pid = fork()))
	{
		perror("fork");
		fail();
	} else if (0 == pid)
	{
		//the child sends a message through the RT message queue
		if (-1 == mq_send(ds, text, strlen(text)+1, PRIORITY))
		{
			perror("mq_send");
			fail();
		}
		if (-1 == mq_close(ds))
		{
			perror("mq_close");
			fail();
		}
		exit(EXIT_SUCCESS);
	}
	//parent receives the message: a thread is called when a message is to be received

	while(0 == msg_recved)//wait for the thread to finish
	{
		sleep(1);
	}

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
