/*
 * streams_pipe.c
 *
 *  Created on: Nov 17, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/un.h>
#include <errno.h>
#include <stddef.h>

int parent_fd[2];
int child_fd[2];

void tell_wait()
{
	//open pipes for each communication channel
	if ((0 > pipe(parent_fd)) || (0 > pipe(child_fd)))
	{
		perror("pipe");
	}
}

void tell_parent()
{
	if (1 != write(child_fd[1], "c", 1))
	{
		perror("tell_parent");
	}
}

void tell_child()
{
	if (1 != write(parent_fd[1], "p", 1))
	{
		perror("tell_child");
	}
}

void wait_parent()
{
	char buf;
	if (1 != read(parent_fd[0], &buf, 1))
	{
		perror("wait_parent");
	}
	if ('p' != buf)
	{
		perror("unknown char");
	}
}

void wait_child()
{
	char buf;
	if (1 != read(child_fd[0], &buf, 1))
	{
		perror("wait_child");
	}
	if ('c' != buf)
	{
		perror("unknown char");
	}
}

#define MSG "my message"
#define SOCKET_NAME "/tmp/my_pipe"
#define MAXLEN 1024

START_TEST (test_template)
{
	pid_t pid;
	int sockfd[2];
	int msg_len = strlen(MSG);
	char line[MAXLEN];
	struct sockaddr_un addr;
	int newfd;
	socklen_t addr_len;

	tell_wait();

	//make sure that this name is not in use
	if (-1 == unlink(SOCKET_NAME))
	{
		perror("unlink");
		fail();
	}

	if (0 > (pid = fork()))
	{
		perror("fork");
		fail();
	} else if (0 == pid)
	{
		//child
		if (-1 == (sockfd[1] = socket(AF_UNIX, SOCK_STREAM, 0)))
		{
			perror("socket");
			fail();
		}
		//name the pipe
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, SOCKET_NAME);
		addr_len = offsetof(struct sockaddr_un, sun_path)+strlen(addr.sun_path);
		if (-1 == bind(sockfd[1], (struct sockaddr*)&addr, addr_len))
		{
			perror("bind");
			fail();
		}
		if (-1 == listen(sockfd[1], 3))
		{
			perror("listen");
			fail();
		}
		tell_parent();

		if (-1 == (newfd = accept(sockfd[1], (struct sockaddr*)&addr, &addr_len)))
		{
			perror("accept");
			fail();
		}

		if (msg_len != write(newfd, MSG, msg_len))
		{
			fprintf(stderr, "Cannot write message : %s\n", strerror(errno));
			fail();
		}
		close(sockfd[1]);
		return;
	}
	//wait child
	wait_child();
	//create a socket
	if (-1 == (sockfd[0] = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		perror("socket");
		fail();
	}
	//open named pipe
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCKET_NAME);
	if (-1 == connect(sockfd[0], (struct sockaddr*)&addr, sizeof(addr)))
	{
		perror("connect");
		fail();
	}

	//then read the message send by the child
	if (msg_len != (read(sockfd[0], line, MAXLEN)))
	{
		fprintf(stderr, "Cannot read message : %s\n", strerror(errno));
		fail();
	}
	fail_unless(strncmp(line, MSG, msg_len) == 0);
	close(sockfd[0]);
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
