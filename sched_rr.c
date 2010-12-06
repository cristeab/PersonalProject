/*
 * sched_rr.c
 *
 *  Created on: Dec 6, 2010
 *      Author: bogdan
 */

#include <sched.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int main()
{
	struct sched_param param;
	struct timespec ts;

	if (-1 == (param.sched_priority = sched_get_priority_max(SCHED_RR)))
	{
		perror("sched_get_priority_max");
		return EXIT_FAILURE;
	}
	if (-1 == sched_setscheduler(0, SCHED_RR, &param))
	{
		perror("sched_setscheduler");
		return EXIT_FAILURE;
	}
	errno = 0;
	nice(-20);
	if (0 != errno)
	{
		perror("nice1");
		return EXIT_FAILURE;
	}
	printf("Current priority = %d\n", getpriority(PRIO_PROCESS, 0));
	if (-1 == sched_rr_get_interval(0, &ts))
	{
		perror("sched_rr_get_interval");
		return EXIT_FAILURE;
	}
	printf("max timeslice = %ld ms\n", ts.tv_nsec/1000000);

	/*if (-1 == sched_setscheduler(0, SCHED_RR, &param))
	{
		perror("sched_setscheduler");
		return EXIT_FAILURE;
	}*/
	errno = 0;
	nice(39);
	if (0 != errno)
	{
		perror("nice2");
		return EXIT_FAILURE;
	}
	printf("Current priority = %d\n", getpriority(PRIO_PROCESS, 0));
	if (-1 == sched_rr_get_interval(0, &ts))
	{
		perror("sched_rr_get_interval");
		return EXIT_FAILURE;
	}
	printf("min timeslice = %ld ms\n", ts.tv_nsec/1000000);
	return EXIT_SUCCESS;
}
