/*
 * sched.c
 *
 *  Created on: Dec 6, 2010
 *      Author: bogdan
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	struct sched_param param, new_param;
	int priority_min = 0;
	int priority_max = 0;

	printf("start policy = %d\n", sched_getscheduler(0));

	priority_min = sched_get_priority_min(SCHED_FIFO);
	priority_max = sched_get_priority_max(SCHED_FIFO);
	param.sched_priority = (priority_min+priority_max)/2;
	printf("min priority = %d, max priority = %d, mean priority = %d\n", priority_min, priority_max, param.sched_priority);

	if (0 != sched_setscheduler(0, SCHED_FIFO, &param))
	{
		perror("sched_setscheduler");
		return EXIT_FAILURE;
	}

	//time critical operations

	//give other processes a chance to run
	sched_yield();

	//change the priority at run time
	param.sched_priority = priority_max;
	if (0 != sched_setparam(0, &param))
	{
		perror("sched_setparam");
		return EXIT_FAILURE;
	}

	//check the priority
	if (-1 == sched_getparam(0, &new_param))
	{
		perror("sched_getparam");
		return EXIT_FAILURE;
	}
	printf("current priority = %d\n", new_param.sched_priority);

	return EXIT_SUCCESS;
}
