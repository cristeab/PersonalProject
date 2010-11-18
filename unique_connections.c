/*
 * unique_connections.c
 *
 *  Created on: Nov 17, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

#define QLEN 10
#define STALE 5

int serv_listen(const char *name)
{
	int fd, len, err, rval;
	struct sockaddr_un un;

	if (NULL == name)
	{
		return -1;
	}

	//create a UNIX domain socket
	errno = 0;
	if (0 > (fd = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		return -2;
	}
	if (-1 == unlink(name))
	{
		return -3;
	}

	//fill the socket address struct
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path)+strlen(name);

	//bind the name to the descriptor
	if (0 > bind(fd, (struct sockaddr*)&un, len))
	{
		rval = -4;
		goto errout;
	}
	if (0 > listen(fd, QLEN))
	{
		rval = -5;
		goto errout;
	}
	return fd;

errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

int serv_accept(int listenfd, uid_t *uidptr)
{
	int clifd, err, rval;
	socklen_t len;
	time_t staletime;
	struct sockaddr_un un;
	struct stat statbuf;

	len = sizeof(un);
	if (0 > (clifd = accept(listenfd, (struct sockaddr*)&un, &len)))
	{
		return EXIT_FAILURE;
	}
	//get the length of the path name
	len -= offsetof(struct sockaddr_un, sun_path);
	un.sun_path[len] = '\0';//null terminate the string

	if (0 > stat(un.sun_path, &statbuf))
	{
		rval = -1;
		goto errout;
	}
	if (0 == S_ISSOCK(statbuf.st_mode))
	{
		rval = -2;
		goto errout;
	}

	if ((statbuf.st_mode & (S_IRWXG | S_IRWXO)) || (statbuf.st_mode & S_IRWXU) != S_IRWXU)
	{
		rval = -3;
		goto errout;
	}

	staletime = time(NULL)-STALE;
	if ((statbuf.st_atime < staletime) || (statbuf.st_ctime < staletime) || (statbuf.st_mtime < staletime))
	{
		rval = -4;
		goto errout;
	}

	if (NULL != uidptr)
	{
		*uidptr = statbuf.st_uid;
	}
	unlink(un.sun_path);//done with this file
	return clifd;

	errout:
	err = errno;
	close(clifd);
	errno = err;
	return rval;
}

#define CLI_PATH "/tmp/sock"
#define CLI_PERM S_IRWXU

int cli_conn(const char *name)
{
	int fd, len, err, rval;
	struct sockaddr_un un;

	if (NULL == name)
	{
		return -1;
	}

	if (0 > (fd = socket(AF_UNIX, SOCK_STREAM, 0)))
	{
		return -2;
	}

	//fill UNIX socket with our own address
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	sprintf(un.sun_path, "%s%05d", CLI_PATH, getpid());
	len = offsetof(struct sockaddr_un, sun_path)+strlen(un.sun_path);

	unlink(un.sun_path);
	if (0 > bind(fd, (struct sockaddr*)&un, len))
	{
		rval = -3;
		goto errout;
	}

	if (0 < chmod(un.sun_path, CLI_PERM))
	{
		rval = -4;
		goto errout;
	}

	//fill socket address with server address
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_UNIX;
	strcpy(un.sun_path, name);
	len = offsetof(struct sockaddr_un, sun_path)+strlen(name);

	if (0 > connect(fd, (struct sockaddr*)&un, len))
	{
		rval = -5;
		goto errout;
	}
	return fd;

	errout:
	err = errno;
	close(fd);
	errno = err;
	return rval;
}

START_TEST (test_serv_listen)
{
	int fd;

	fd = serv_listen(NULL);
	fail_unless(-1 == fd);

	fd = serv_listen("");
	fail_unless(-3 == fd);

	//create a file
	unlink("server");//make sure the file doesn't exist
	if (-1 == open("server", O_RDONLY | O_CREAT))
	{
		fail();
	}

	fd = serv_listen("server");
	fail_unless(fd > 0);
	unlink("server");
}
END_TEST

START_TEST (test_client)
{
	int fd_srv, fd_client, fd_client2;
	uid_t uid = 0;

	//create a file
	unlink("server");//make sure the file doesn't exist
	if (-1 == open("server", O_RDONLY | O_CREAT))
	{
		fail();
	}

	//setup server
	fd_srv = serv_listen("server");
	fail_unless(fd_srv > 0);

	//connect to server
	fd_client = cli_conn("server");
	fail_unless(fd_client > 0);
	fail_unless(fd_client != fd_srv);

	//try to accept a connection without a request (should wait forever)

	//accept connection
	fd_client2 = serv_accept(fd_srv, &uid);
	fail_unless(fd_client2 > 0);
	fail_unless(uid == getuid());
	fail_unless(fd_client2 != fd_client);
	fail_unless(fd_client2 != fd_srv);

	unlink("server");
}
END_TEST

#define MSG "my message"
#define BUFFER_LEN 1024

START_TEST (test_communication)
{
	pid_t pid;
	uid_t uid;
	int status;
	int fd_cli, fd_srv, fd_srv_unique;
	char line[BUFFER_LEN+1];
	ssize_t line_len;

	unlink("server");//make sure the file doesn't exist
	if (-1 == open("server", O_RDONLY | O_CREAT))
	{
		fail();
	}

	//setup server
	fd_srv = serv_listen("server");
	fail_unless(fd_srv > 0);

	if (0 > (pid = fork()))
	{
		fail();
	} else if (0 == pid)
	{
		//child
		fd_cli = cli_conn("server");
		fail_unless(fd_cli > 0);
		line_len = write(fd_cli, MSG, strlen(MSG));
		fail_unless(line_len == strlen(MSG));
		return;
	}
	//parent
	fd_srv_unique = serv_accept(fd_srv, &uid);
	fail_unless(fd_srv_unique > 0);

	line_len = read(fd_srv_unique, line, BUFFER_LEN);
	line[line_len] = '\0';
	printf("%s\n", line);
	fail_unless(0 == strcmp(line, MSG));

	waitpid(pid, &status, 0);
	unlink("server");
}
END_TEST

Suite* test_suite(void)
{
        Suite *s = suite_create("named_conn_suite");

        /* Core test case */
        TCase *tc_core = tcase_create("Core");
        tcase_add_test(tc_core, test_serv_listen);
        tcase_add_test(tc_core, test_client);
        tcase_add_test(tc_core, test_communication);
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
