/*
 * hello.c
 *
 *  Created on: Dec 6, 2010
 *      Author: bogdan
 */

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <stdlib.h>

#define __rt_text __attribute__ ((__section__ ("real_text")))
#define __rt_data __attribute__ ((__section__ ("real_data")))
#define __rt_bss __attribute__ ((__section__ ("real_bss")))

extern unsigned int __start_real_text, __stop_real_text;
extern unsigned int __start_real_data, __stop_real_data;
extern unsigned int __start_real_bss, __stop_real_bss;

char rt_bss[100] __rt_bss;
char rt_data[] __rt_data = "Hello Real-Time World\n";

void __rt_text hello_rt_world()
{
	extern char rt_data[], rt_bss[100];

	printf("%s", rt_data);
	memset(rt_bss, 0xFF, sizeof(rt_bss));
}

void rt_lockall()
{
	mlock(&__start_real_text, &__stop_real_text-&__start_real_text);
	mlock(&__start_real_data, &__stop_real_data-&__start_real_data);
	mlock(&__start_real_bss, &__stop_real_bss-&__start_real_bss);
}

void hello_world()
{
	printf("hello\n");
}

int main()
{
	rt_lockall();
	hello_world();
	hello_rt_world();
	return EXIT_SUCCESS;
}
