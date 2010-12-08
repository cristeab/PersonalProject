/*
 * mlock.c
 *
 *  Created on: Dec 6, 2010
 *      Author: bogdan
 */

#include <sys/mman.h>
#include <unistd.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#define RT_BUFSIZE 1024

int main()
{
	unsigned long pagesize, offset;
	char *rt_buffer = malloc(RT_BUFSIZE);

	if (NULL == rt_buffer)
	{
		fprintf(stderr, "Cannot allocate heap memory\n");
		return EXIT_FAILURE;
	}

	if (-1 == (pagesize = sysconf(_SC_PAGESIZE)))
	{
		perror("sysconf");
		return EXIT_FAILURE;
	}
	printf("pagesize = %ld\n", pagesize);
	offset = (unsigned long)rt_buffer%pagesize;
	if (0 != mlock(rt_buffer-offset, RT_BUFSIZE+offset))
	{
		perror("mlock");
		return EXIT_FAILURE;
	}

	//don't worry about latencies due to paging

	//after use unlock
	if (0 != munlock(rt_buffer-offset, RT_BUFSIZE+offset))
	{
		perror("munlock");
		return EXIT_FAILURE;
	}

	//lock the entire process memory as well as future memory allocations
	if (0 != mlockall(MCL_CURRENT | MCL_FUTURE))
	{
		perror("mlockall");
		return EXIT_FAILURE;
	}

	rt_buffer = realloc(rt_buffer, 2*RT_BUFSIZE);
	if (NULL == rt_buffer)
	{
		fprintf(stderr, "Cannot reallocate memory\n");
		return EXIT_FAILURE;
	}

	if (0 != munlockall())
	{
		perror("munlockall");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
