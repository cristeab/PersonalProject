/*
 * hello_main.c
 *
 *  Created on: Dec 6, 2010
 *      Author: bogdan
 */

#include <stdio.h>
#include <stdlib.h>
extern void hello_world();
extern void hello_rt_world();

extern unsigned long __start_rt_text, __end_rt_text;
extern unsigned long __start_rt_data, __end_rt_data;
extern unsigned long __start_rt_bss, __end_rt_bss;

void rt_lockall()
{
	mlock(&__start_rt_text, &__end_rt_text-&__start_rt_text);
	mlock(&__start_rt_data, &__end_rt_data-&__start_rt_data);
	mlock(&__start_rt_bss, &__end_rt_bss-&__start_rt_bss);
}

int main()
{
	rt_lockall();
	hello_world();
	hello_rt_world();
	return EXIT_SUCCESS;
}
