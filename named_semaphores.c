/*
 * named_semaphores.c
 *
 *  Created on: Dec 8, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include <semaphore.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

#define SEM_NAME "/my_semaphore"
START_TEST (test_template)
{
	sem_t *sema_n;
	int ret, val;

	sem_unlink(SEM_NAME);//make sure that this semaphore does not exist
	if (SEM_FAILED == (sema_n = sem_open(SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR, 1)))
	{
		perror("sem_open");
		fail();
	}

	if (0 != sem_getvalue(sema_n, &val))
	{
		perror("sem_getvalue");
		fail();
	}
	fail_unless(1 == val);

	//acquire semaphore
	errno = 0;
	if (0 != (ret = sem_trywait(sema_n)) && EAGAIN == errno)
	{
		if (0 != sem_wait(sema_n))
		{
			perror("sem_wait");
			fail();
		}
	} else if (0 != errno)
	{
		perror("sem_trywait");
		fail();
	}

	//operate on shared data

	//release semaphore
	if (0 != sem_post(sema_n))
	{
		perror("sem_post");
		fail();
	}
	if (0 != sem_close(sema_n))
	{
		perror("sem_close");
		fail();
	}
	if (0 != sem_unlink(SEM_NAME))
	{
		perror("sem_unlink");
		fail();
	}
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
