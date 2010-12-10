/*
 * test_character_dd.c
 *
 *  Created on: Dec 10, 2010
 *      Author: bogdan
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd;
	int rc = 0;
	char *rd_buf[16];

	printf("%s: entered\n", argv[0]);
	if (-1 == (fd = open("/dev/hello1", O_RDWR)))
	{
		perror("open");
		return EXIT_FAILURE;
	}
	printf("%s: open: successful\n", argv[0]);
	if (0 > (rc = read(fd, rd_buf, 0)))
	{
		perror("read");
		return EXIT_FAILURE;
	}
	printf("%s: read: returning %d bytes\n", argv[0], rc);
	if (-1 == close(fd))
	{
		perror("close");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
