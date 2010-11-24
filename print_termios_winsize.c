#include <termios.h>
/*
 * print_termios_winsize.c
 *
 *  Created on: Nov 24, 2010
 *      Author: bogdan
 */

#include <termios.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
	struct termios tios;
	struct winsize wsize;

	//print termios
	if (0 > tcgetattr(STDIN_FILENO, &tios))
	{
		perror("tcgetattr");
		return EXIT_FAILURE;
	}
	printf("Input mode: ");
	if (IGNBRK & tios.c_iflag)
	{
		printf("Ignore BREAK condition on input\n");
	}
	if (BRKINT & tios.c_iflag)
	{
		printf("If IGNBRK is set, a BREAK is ignored\n");
	}
	if (IGNPAR & tios.c_iflag)
	{
		printf("Ignore framing errors and parity errors\n");
	}
	if (PARMRK & tios.c_iflag)
	{
		printf("If  IGNPAR  is  not  set, prefix a character with a parity error\n");
	}
	if (INPCK & tios.c_iflag)
	{
		printf("Enable input parity checking\n");
	}
	if (ISTRIP & tios.c_iflag)
	{
		printf("Strip off eighth bit\n");
	}
	if (INLCR & tios.c_iflag)
	{
		printf("Ignore carriage return on input\n");
	}
	if (ICRNL & tios.c_iflag)
	{
		printf("Translate carriage return to newline on input (unless IGNCR is set)\n");
	}
	if (IUCLC & tios.c_iflag)
	{
		printf("(not in POSIX) Map uppercase characters to lowercase on input\n");
	}
	if (IXON & tios.c_iflag)
	{
		printf("Enable XON/XOFF flow control on output\n");
	}
	if (IXANY & tios.c_iflag)
	{
		printf("Typing any character will restart stopped output\n");
	}
	if (IXOFF & tios.c_iflag)
	{
		printf("Enable XON/XOFF flow control on input\n");
	}
	if (IMAXBEL & tios.c_iflag)
	{
		printf("Ring bell when input queue is full\n");
	}
	if (IUTF8 & tios.c_iflag)
	{
		printf("Input is UTF8\n");
	}
	printf("Output mode: %d\n", tios.c_oflag);
	printf("Control modes: %d\n", tios.c_cflag);
	printf("Local modes: %d\n", tios.c_lflag);

	//print winsize
	if (0 > ioctl(STDIN_FILENO, TIOCGWINSZ, (char*)&wsize))
	{
		perror("ioctl");
		return EXIT_FAILURE;
	}
	printf("Column: %d\n", wsize.ws_col);
	printf("Row: %d\n", wsize.ws_row);
	printf("XPixel: %d\n", wsize.ws_xpixel);
	printf("YPixel: %d\n", wsize.ws_ypixel);

	return EXIT_SUCCESS;
}
