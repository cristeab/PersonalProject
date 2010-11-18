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
#include <stddef.h>
#include <sys/un.h>

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
			fprintf(stderr, "Error when allocating memory for control message\n");
			return -1;
		}
		msg.msg_control = cmsgptr;
		msg.msg_controllen = CMSG_LEN(sizeof(int));
		if (0 > (nr = recvmsg(fd, &msg, 0)))
		{
			perror("recvmsg");
			return -2;
		} else if (0 == nr) {
			fprintf(stderr, "Connection closed by server\n");
			return -3;
		}

		for (ptr = buf; ptr < &buf[nr];)
		{
			if (*ptr++ == 0)
			{
				if (ptr != &buf[nr-1])
				{
					fprintf(stderr, "message format error\n");
					return -4;
				}
				status = *ptr & 0xFF;
				if (0 == status)
				{
					if (msg.msg_controllen == 0)
					{
						fprintf(stderr, "status is 0 but no fd\n");
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
			fprintf(stderr, "Error in userfunc\n");
			return -6;
		}
		if (status >= 0)
		{
			return newfd;
		}
	}
}

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

#define SERVER "/tmp/my_server"
int csopen(char *name, int oflag)
{
	pid_t pid;
	int len;
	char buf[10];
	struct iovec iov[3];
	static int fd[] = {-1, -1};
	struct sockaddr_un addr;
	socklen_t addr_len;
	int new_fd = -1;

	//init server address
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SERVER);
	addr_len = offsetof(struct sockaddr_un, sun_path)+strlen(addr.sun_path);

	if (0 > new_fd)
	{
		tell_wait();
		if (0 > (pid = fork()))
		{
			perror("fork");
			return EXIT_FAILURE;
		} else if (0 == pid)
		{
			//open socket and connect to server
			if (-1 == (fd[1] = socket(AF_UNIX, SOCK_STREAM, 0)))
			{
				perror("socket child");
				return EXIT_FAILURE;
			}
			wait_parent();
			if (-1 == connect(fd[1], (struct sockaddr*)&addr, addr_len))
			{
				perror("connect");
				return EXIT_FAILURE;
			}

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
		//create a server and accept connections on it
		if (-1 == (fd[0] = socket(AF_UNIX, SOCK_STREAM, 0)))
		{
			perror("socket parent");
			return EXIT_FAILURE;
		}
		if (-1 == unlink(SERVER))
		{
			perror("unlink");
			return EXIT_FAILURE;
		}
		if (-1 == bind(fd[0], (struct sockaddr*)&addr, addr_len))
		{
			perror("bind");
			return EXIT_FAILURE;
		}
		if (-1 == listen(fd[0], 3))
		{
			perror("listen");
			return EXIT_FAILURE;
		}
		tell_child();
		if (-1 == (new_fd = accept(fd[0], (struct sockaddr*)&addr, &addr_len)))
		{
			perror("accept");
			return EXIT_FAILURE;
		}
	}
	sprintf(buf, " %d", oflag);
	iov[0].iov_base = CL_OPEN " ";//string concatenation
	iov[0].iov_len = strlen(CL_OPEN)+1;
	iov[1].iov_base = name;
	iov[1].iov_len = strlen(name);
	iov[2].iov_base = buf;
	iov[2].iov_len = strlen(buf)+1;
	len = iov[0].iov_len+iov[1].iov_len+iov[2].iov_len;
	if (len != writev(new_fd, iov, 3))
	{
		perror("writev");
		return EXIT_FAILURE;
	}

	return recv_fd(new_fd, write);
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
