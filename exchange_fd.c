/*
 * exchange_fd.c
 *
 *  Created on: Nov 18, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/un.h>
#include <stddef.h>

static struct cmsghdr *cmsgptr = NULL;

int send_fd(int fd, int fd_to_send)
{
	struct iovec iov[1];
	struct msghdr msg;
	char buf[2];

	iov[0].iov_base = buf;
	iov[0].iov_len = 2;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	if (fd_to_send < 0)
	{
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		buf[1] = -fd_to_send;
		if (0 == buf[1])
		{
			buf[1] = 1;
		}
	} else {
		//allocate memory for control message
		if ((NULL == cmsgptr) && (NULL == (cmsgptr = malloc(CMSG_LEN(sizeof(int))))))
		{
			return -1;
		}
		cmsgptr->cmsg_level = SOL_SOCKET;
		cmsgptr->cmsg_type = SCM_RIGHTS;
		cmsgptr->cmsg_len = CMSG_LEN(sizeof(int));
		*(int*)CMSG_DATA(cmsgptr) = fd_to_send;
		msg.msg_control = cmsgptr;
		msg.msg_controllen = cmsgptr->cmsg_len;
		buf[1] = 0;
	}
	buf[0] = 0;
	if (2 != sendmsg(fd, &msg, 0))
	{
		return -2;
	}
	return 0;
}

#define MAXLINE 1024
int recv_fd(int fd, ssize_t (*userfunc)(int, const void*, size_t))
{
	int newfd = -1, nr, status;
	char *ptr;
	char buf[MAXLINE];
	struct iovec iov[1];
	struct msghdr msg;

	status = -1;
	while (1)
	{
		iov[0].iov_base = buf;
		iov[0].iov_len = MAXLINE;
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		if ((NULL == cmsgptr) && (NULL == (cmsgptr = malloc(CMSG_LEN(sizeof(int))))))
		{
			return -1;
		}
		msg.msg_control = cmsgptr;
		msg.msg_controllen = CMSG_LEN(sizeof(int));
		if (0 > (nr = recvmsg(fd, &msg, 0)))
		{
			//perror("recvmsg");
			return -2;
		} else if (0 == nr) {
			//fprintf(stderr, "Connection closed by server\n");
			return -3;
		}

		for (ptr = buf; ptr < &buf[nr];)
		{
			if (*ptr++ == 0)
			{
				if (ptr != &buf[nr-1])
				{
					//fprintf(stderr, "message format error\n");
					return -4;
				}
				status = *ptr & 0xFF;
				if (0 == status)
				{
					if (msg.msg_controllen != CMSG_LEN(sizeof(int)))
					{
						//fprintf(stderr, "status is 0 but no fd\n");
						return -5;
					}
					newfd = *(int*)CMSG_DATA(cmsgptr);
				} else {
					newfd = -status;
				}
				nr -= 2;
			}
		}
		if (nr > 0 && (*userfunc)(STDERR_FILENO, buf, nr) != nr)
		{
			return -6;
		}
		if (status >= 0)
		{
			return newfd;
		}
	}
}

#define SOCK_NAME "/tmp/socket"
START_TEST (test_send_fd)
{
	int sockfd = -1;
	int dummy_fd = STDERR_FILENO;
	struct sockaddr_un addr;
	socklen_t addr_len = 0;
	int conn_fd;
	pid_t pid;

	//send fd
	fail_unless(send_fd(sockfd, dummy_fd) == -2);

	//create server address
	if (-1 == unlink(SOCK_NAME))
	{
		fail();
	}
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCK_NAME);
	addr_len = offsetof(struct sockaddr_un, sun_path)+strlen(SOCK_NAME);

	//create a child in order to connect to from the other end of the pipe
	if (0 > (pid = fork()))
	{
		fail();
	} else if (0 == pid)
	{
		//child
		if (-1 == (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)))
		{
			fail();
		}
		if (-1 == connect(sockfd, (struct sockaddr *)&addr, addr_len))
		{
			fail();
		}
		return;
	}
	//parent
	if (-1 == (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		fail();
	}
	//bind socket to an address
	if (-1 == bind(sockfd, (struct sockaddr *)&addr, addr_len))
	{
		perror("bind");
		fail();
	}

	if (-1 == listen(sockfd, 3))
	{
		perror("listen");
		fail();
	}
	//accept connections using this end
	if (-1 == (conn_fd = accept(sockfd, (struct sockaddr *)&addr, &addr_len)))
	{
		fail();
	}
	//try to send again the fd
	fail_unless(send_fd(conn_fd, dummy_fd) == 0);
}
END_TEST

START_TEST (test_send_fd_with_sockpair)
{
	int sockfd[2];
	int dummy_fd = STDERR_FILENO;

	if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd))
	{
		perror("socketpair");
		fail();
	}

	fail_unless(send_fd(sockfd[0], dummy_fd) == 0);

	//should fail if the other end is closed
	close(sockfd[1]);
	fail_unless(send_fd(sockfd[0], dummy_fd) == -2);
}
END_TEST

START_TEST (test_fd_exchange)
{
	int sockfd = -1;
	int dummy_fd = STDERR_FILENO;
	struct sockaddr_un addr;
	socklen_t addr_len = 0;
	int conn_fd;
	pid_t pid;

	//send fd
	fail_unless(send_fd(sockfd, dummy_fd) == -2);

	//create server address
	if (-1 == unlink(SOCK_NAME))
	{
		fail();
	}
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SOCK_NAME);
	addr_len = offsetof(struct sockaddr_un, sun_path)+strlen(SOCK_NAME);

	//create a child in order to connect to from the other end of the pipe
	if (0 > (pid = fork()))
	{
		fail();
	} else if (0 == pid)
	{
		//child
		if (-1 == (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)))
		{
			fail();
		}
		if (-1 == connect(sockfd, (struct sockaddr *)&addr, addr_len))
		{
			fail();
		}
		fail_unless(recv_fd(sockfd, write) == STDERR_FILENO);
		return;
	}
	//parent
	if (-1 == (sockfd = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		fail();
	}
	//bind socket to an address
	if (-1 == bind(sockfd, (struct sockaddr *)&addr, addr_len))
	{
		perror("bind");
		fail();
	}

	if (-1 == listen(sockfd, 3))
	{
		perror("listen");
		fail();
	}
	//accept connections using this end
	if (-1 == (conn_fd = accept(sockfd, (struct sockaddr *)&addr, &addr_len)))
	{
		fail();
	}
	//send the fd
	fail_unless(send_fd(conn_fd, dummy_fd) == 0);
}
END_TEST

START_TEST (test_fd_exchange_with_sockpair)
{
	int sockfd[2];
	int dummy_fd = STDERR_FILENO;

	if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd))
	{
		perror("socketpair");
		fail();
	}

	//send fd from one end
	fail_unless(send_fd(sockfd[0], dummy_fd) == 0);

	//receive fd from the other end (doesn't work)
	fail_unless(recv_fd(sockfd[1], write) == -5);
}
END_TEST

Suite* test_suite(void)
{
        Suite *s = suite_create("exchange_fd");

        /* Core test case */
        TCase *tc_core = tcase_create("Core");
        tcase_add_test(tc_core, test_send_fd);
        tcase_add_test(tc_core, test_send_fd_with_sockpair);
        tcase_add_test(tc_core, test_fd_exchange);
        tcase_add_test(tc_core, test_fd_exchange_with_sockpair);
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
