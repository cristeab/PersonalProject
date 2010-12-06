/*
 * shm.c
 *
 *  Created on: Dec 6, 2010
 *      Author: bogdan
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define SHM_SIZE 1024

int main()
{
	int shm_fd;
	void *vaddr;

	if (-1 == (shm_fd = shm_open("my_shm", O_CREAT | O_RDWR, 0666)))
	{
		perror("shm_open");
		return EXIT_FAILURE;
	}

	//set shmem size
	if (0 != ftruncate(shm_fd, SHM_SIZE))
	{
		perror("ftruncate");
		return EXIT_FAILURE;
	}

	//map shm to addr space
	if (MAP_FAILED == (vaddr = mmap(0, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0)))
	{
		perror("mmap");
		return EXIT_FAILURE;
	}

	if (0 != mlock(vaddr, SHM_SIZE))
	{
		perror("mlock");
		return EXIT_FAILURE;
	}

	write(shm_fd, "something\n", strlen("something\n")+1);

	//unmapping the memory also unlocks it
	if (-1 == munmap(vaddr, SHM_SIZE))
	{
		perror("munmap");
		return EXIT_FAILURE;
	}
	close(shm_fd);
	shm_unlink("my_shm");
	return EXIT_SUCCESS;
}
