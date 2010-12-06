/*
 * hello_rt_world.c
 *
 *  Created on: Dec 6, 2010
 *      Author: bogdan
 */

#include <stdio.h>
#include <string.h>

void hello_rt_world()
{
	extern char rt_data[], rt_bss[100];

	printf("%s", rt_data);
	memset(rt_bss, 0xFF, sizeof(rt_bss));
}
