/*
 * opend1.c
 *
 *  Created on: Nov 18, 2010
 *      Author: bogdan
 */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAXLINE 1024
#define CL_OPEN "open"

char errmsg[MAXLINE];
int oflag;
char *pathname;

#define MAXARGC 50
#define WHITE " \t\n"
int buf_args(char *buf, int (*optfunc)(int, char **))
{
	char *ptr, *argv[MAXARGC];
	int argc;

	if (NULL == strtok(buf, WHITE))
	{
		return -1;
	}
	argv[argc = 0] = buf;
	while (NULL != (ptr = strtok(NULL, WHITE)))
	{
		if (++argc >= MAXARGC-1)
		{
			return -1;
		}
		argv[argc] = ptr;
	}
	argv[++argc] = NULL;
	return (*optfunc)(argc, argv);
}

int cli_args(int argc, char **argv)
{
	if (3 != argc || strcmp(argv[0], CL_OPEN))
	{
		fprintf(stderr, "usage: <pathname><oflag>\n");
		return -1;
	}
	pathname = argv[1];
	oflag = atoi(argv[2]);
	return 0;
}

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
		fprintf(stderr, "error in sendmsg\n");
		return -2;
	}
	return 0;
}

void request(char *buf, int nread, int fd)
{
	int newfd;

	if (0 != buf[nread-1])
	{
		//don't send error message back through socket
		fprintf(stderr, "the received buffer is not zero terminated\n");
		return;
	}
	if (0 > buf_args(buf, cli_args))
	{
		//don't sent error message
		fprintf(stderr, "error in buf_args\n");
		return;
	}
	if (0 > (newfd = open(pathname, oflag)))
	{
		perror("open");
		return;
	}
	if (0 > send_fd(fd, newfd))
	{
		fprintf(stderr, "send_fd error\n");
		return;
	}
	close(newfd);
}

int main()
{
	int nread;
	char buf[MAXLINE];

	while (1)
	{
		if (0 > (nread = read(STDIN_FILENO, buf, MAXLINE)))
		{
			perror("read");
			return EXIT_FAILURE;
		} else if (0 == nread)
		{
			break;
		}
		request(buf, nread, STDOUT_FILENO);
	}
	return EXIT_SUCCESS;
}
