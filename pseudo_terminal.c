/*
 * pseudo_terminal.c
 *
 *  Created on: Nov 24, 2010
 *      Author: bogdan
 */

#include <check.h>
#define __USE_XOPEN
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

int ptym_open(char *pts_name, int pts_namesz)
{
	char *ptr = NULL;
	int fdm;

	strncpy(pts_name, "/dev/ptyXX", pts_namesz);
	pts_name[pts_namesz-1] = '\0';
	if (0 > (fdm = posix_openpt(O_RDWR)))
	{
		return -1;
	}
	if (0 > grantpt(fdm))
	{
		close (fdm);
		return -2;
	}
	if (0 > unlockpt(fdm))
	{
		close(fdm);
		return -3;
	}
	if (NULL == (ptr = ptsname(fdm)))
	{
		close(fdm);
		return -4;
	}

	strncpy(pts_name, ptr, pts_namesz);
	pts_name[pts_namesz-1] = '\0';
	return fdm;
}

int ptys_open(char *pts_name)
{
	int fds;

	if (0 > (fds = open(pts_name, O_RDWR)))
	{
		return -5;
	}
	return fds;
}

#define BUF_LEN 128
START_TEST (test_template)
{
	char buf[BUF_LEN];

	//open master pts
	fail_unless(ptym_open(buf, BUF_LEN)>0);
	fail_unless(strncmp(buf, "/dev/pts", 8) == 0);

	//open slave pts
	fail_unless(ptys_open(buf) > 0);
}
END_TEST

Suite* test_suite(void)
{
        Suite *s = suite_create("templated_suite");

        /* Core test case */
        TCase *tc_core = tcase_create("Core");
        tcase_add_test(tc_core, test_template);
        suite_add_tcase(s, tc_core);

        return s;
}

int main()
{
        int number_failed;
        Suite *s = test_suite();
        SRunner *sr = srunner_create(s);
        srunner_run_all(sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(sr);
        srunner_free(sr);
        return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
