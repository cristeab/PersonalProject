/*
 * archive_data.c
 *
 *  Created on: Nov 11, 2010
 *      Author: bogdan
 */

#include <stdlib.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <strings.h>

#define SRC_FILE "/home/bogdan/Untitled.jpg"
#define DST_FILE "dump.bin"
#define MAX_BUFFER_LEN (1<<20)
#define MAX_LEN (330*(1<<20))

int main(int argc, char *argv[])
{
	int fd;
	void *pBuffer = NULL;
	struct timeval tv_start;
	struct timeval tv_stop;
	struct timeval tv_res;
	struct sched_param param;
	ssize_t len = 0;
	ssize_t actually_written = 0;
	struct stat file_stat;
	int option = 0;
	unsigned char prioritize = 0;
	int policy = 0;
	int nb_seq = 1;
	int n = 0;

	if (-1 == (fd = open(SRC_FILE, O_RDONLY)))
	{
		perror("open");
		return EXIT_FAILURE;
	}

	if (MAP_FAILED == (pBuffer = mmap(NULL, MAX_BUFFER_LEN, PROT_READ, MAP_PRIVATE, fd, 0)))
	{
		perror("mmap");
		return EXIT_FAILURE;
	}
	close(fd);

	if (-1 == (fd = open(DST_FILE, O_WRONLY | O_CREAT)))
	{
		perror("open");
		return EXIT_FAILURE;
	}

	if (-1 == unlink(DST_FILE))
	{
		perror("unlink");
		return EXIT_FAILURE;
	}

	if (0 != posix_fallocate(fd, 0, MAX_LEN))
	{
		fprintf(stderr, "posix_fallocate\n");
		return EXIT_FAILURE;
	}
	if (0 != posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL | POSIX_FADV_WILLNEED))
	{
		fprintf(stderr, "posix_fadvise\n");
		return EXIT_FAILURE;
	}

	while (-1 != (option = getopt(argc, argv, "p:s:")))
	{
		switch (option)
		{
		case 'p':
			prioritize = 1;
			if (0 == strcasecmp(optarg, "fifo"))
			{
				policy = SCHED_FIFO;
				printf("Using FIFO scheduling policy\n");
			} else if (0 == strcasecmp(optarg, "rr"))
			{
				policy = SCHED_RR;
				printf("Using Round-Robin scheduling policy\n");
			} else
			{
				fprintf(stderr, "unknown policy");
				return EXIT_FAILURE;
			}
			break;
		case 's':
			if (0 >= (nb_seq = atoi(optarg)))
			{
				fprintf(stderr, "sequence no should be striclty positive\n");
				return EXIT_FAILURE;
			}
			break;
		default:
			return EXIT_FAILURE;
		}
	}
	if (1 == prioritize)
	{
		if (-1 == (param.sched_priority = sched_get_priority_max(policy)))
		{
			perror("sched_get_priority_max");
			return EXIT_FAILURE;
		}
		if (-1 == sched_setscheduler(0, policy, &param))
		{
			perror("sched_setscheduler");
			return EXIT_FAILURE;
		}
	} else
	{
		printf("Using default scheduler\n");
	}

	printf("Using %d sequence(s)\n", nb_seq);

	if (-1 == gettimeofday(&tv_start, NULL))
	{
		perror("gettimeofday");
		return EXIT_FAILURE;
	}
	for (n = 0; n < nb_seq; ++n)
	{
		len = MAX_LEN;
		do
		{
			if (-1 == (actually_written = write(fd, pBuffer, (len
					>MAX_BUFFER_LEN) ? MAX_BUFFER_LEN : len)))
			{
				perror("write");
				return EXIT_FAILURE;
			}
			len -= actually_written;
		} while (len > 0);
		if ((n+1)%8)
		{
			fsync(fd);
		}
	}
	fsync(fd);
	if (-1 == gettimeofday(&tv_stop, NULL))
	{
		perror("gettimeofday");
		return EXIT_FAILURE;
	}

	if (-1 == fstat(fd, &file_stat))
	{
		perror("fstat");
		return EXIT_FAILURE;
	}
	timersub(&tv_stop, &tv_start, &tv_res);
	printf("Written %ld MB in \n", file_stat.st_size/(1<<20));

	printf("Elapsed time: %ld s and %ld ms\n", (long)tv_res.tv_sec/nb_seq, (long)tv_res.tv_usec/(1000*nb_seq));

	close(fd);
	return EXIT_SUCCESS;
}
