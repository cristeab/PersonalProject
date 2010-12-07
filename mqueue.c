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

#define QUEUE_NAME "/my_queue"
#define PRIORITY 1
#define SIZE 256

START_TEST (test_template)
{
	pid_t pid;
	mqd_t ds;
	const char text[] = "Hello RT POSIX World";
	char recv_text[SIZE];
	struct mq_attr queue_attr, queue_old_attr;
	unsigned int prio;

	queue_attr.mq_maxmsg = 32;
	queue_attr.mq_msgsize = SIZE;
	queue_attr.mq_flags = O_NONBLOCK;
	queue_attr.mq_curmsgs = 0;

	//open queue for read/write
	if ((mqd_t)-1 == (ds = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &queue_attr)))
	{
		perror("mq_open");
		fail();
	}

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
		exit(EXIT_SUCCESS);
	}
	//parent receives the message
	queue_attr.mq_flags = 0;//!O_NONBLOCK
	if ((mqd_t)-1 == mq_setattr(ds, &queue_attr, NULL))
	{
		perror("mq_setattr");
		fail();
	}
	if ((mqd_t)-1 == mq_getattr(ds, &queue_old_attr))
	{
		perror("mq_getattr");
		fail();
	}
	if (queue_old_attr.mq_flags & O_NONBLOCK)
	{
		fprintf(stderr, "O_NONBLOCK should not be set\n");
		fail();
	}
	//wait till a message is received
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
