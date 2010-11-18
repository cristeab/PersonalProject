/*
 * client1.c
 *
 *  Created on: Nov 18, 2010
 *      Author: bogdan
 */

#define CL_OPEN "open"
#define BUFFSIZE 8192
#define MAXLINE 1024
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>

static struct cmsghdr *cmsgptr = NULL;

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

int csopen(char *name, int oflag)
{
	pid_t pid;
	int len;
	char buf[10];
	struct iovec iov[3];
	static int fd[] = {-1, -1};

	if (0 > fd[0])
	{
		if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, fd))
		{
			perror("socketpair");
			return EXIT_FAILURE;
		}
		if (0 > (pid = fork()))
		{
			perror("fork");
			return EXIT_FAILURE;
		} else if (0 == pid)
		{
			close(fd[0]);
			if (fd[1] != STDIN_FILENO && dup2(fd[1], STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2 on stdin");
				return EXIT_FAILURE;
			}
			if (fd[1] != STDOUT_FILENO && dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO)
			{
				perror("dup2 on stdin");
				return EXIT_FAILURE;
			}
			if (0 > execl("bin/opend1", "opend1", (char*)0))
			{
				perror("execl");
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		}
		close(fd[1]);
	}
	sprintf(buf, " %d", oflag);
	iov[0].iov_base = CL_OPEN " ";//string concatenation
	iov[0].iov_len = strlen(CL_OPEN)+1;
	iov[1].iov_base = name;
	iov[1].iov_len = strlen(name);
	iov[2].iov_base = buf;
	iov[2].iov_len = strlen(buf)+1;
	len = iov[0].iov_len+iov[1].iov_len+iov[2].iov_len;
	if (len != writev(fd[0], iov, 3))
	{
		perror("writev");
		return EXIT_FAILURE;
	}

	return recv_fd(fd[0], write);
}

int main(int argc, char *argv[])
{
	int n, fd;
	char buf[BUFFSIZE], line[MAXLINE];

	//read file to cat from stdin
	while (NULL != fgets(line, MAXLINE, stdin))
	{
		if ('\n' == line[strlen(line)-1])
		{
			line[strlen(line)-1] = '\0';
		}
		if (0 > (fd = csopen(line, O_RDONLY)))
		{
			fprintf(stderr, "csopen failed\n");
			continue;
		}
		//cat file to std out
		while (0 < (n = read(fd, buf, BUFFSIZE)))
		{
			if (n != write(STDOUT_FILENO, buf, n))
			{
				perror("write");
				return EXIT_FAILURE;
			}
			close(fd);
		}
	}
	return EXIT_SUCCESS;
}
