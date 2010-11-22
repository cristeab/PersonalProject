/*
 * ex_get_pass.c
 *
 *  Created on: Nov 22, 2010
 *      Author: bogdan
 */

#define MAX_PASS_LEN 8
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

char* getpass(const char *prompt)
{
	static char buf[MAX_PASS_LEN+1];
	char *ptr;
	sigset_t sig, osig;
	struct termios ts, ots;
	FILE *fp;
	int c;

	if (NULL == (fp = fopen(ctermid(NULL), "r+")))
	{
		perror("fopen");
		return NULL;
	}
	setbuf(fp, buf);

	sigemptyset(&sig);
	sigaddset(&sig, SIGINT);
	sigaddset(&sig, SIGTSTP);
	sigprocmask(SIG_BLOCK, &sig, &osig);

	tcgetattr(fileno(fp), &ts);
	ots = ts;
	ts.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
	tcsetattr(fileno(fp), TCSAFLUSH, &ts);
	fputs(prompt, fp);

	ptr = buf;
	while ((EOF != (c = getc(fp))) && (c != '\n'))
	{
		if (ptr < &buf[MAX_PASS_LEN])
		{
			*ptr++ = c;
		}
	}
	*ptr = 0;
	putc('\n', fp);

	tcsetattr(fileno(fp), TCSAFLUSH, &ots);
	sigprocmask(SIG_SETMASK, &osig, NULL);
	fclose(fp);
	return buf;
}

int main()
{
	char *ptr;

	if (NULL == (ptr = getpass("Enter password: ")))
	{
		fprintf(stderr, "Error in getpass\n");
		return EXIT_FAILURE;
	}
	printf("Password: %s\n", ptr);

	//zero out the memory
	while (*ptr != 0)
	{
		*ptr++ = 0;
	}
	return EXIT_SUCCESS;
}
