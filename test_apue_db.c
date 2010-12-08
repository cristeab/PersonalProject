/*
 * test_apue_db.c
 *
 *  Created on: Nov 29, 2010
 *      Author: bogdan
 */

#include <check.h>
#include <stdlib.h>
#include "apue_db.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>

#define DB_NAME "test_db"

void setup()
{
	unlink(DB_NAME ".dat");
	unlink(DB_NAME ".idx");
}

void teardown()
{
}

START_TEST (test_open)
{
	DBHANDLE h;
	h = db_open(NULL, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	fail_unless(NULL == h);
	h = db_open(DB_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	fail_unless(NULL != h);
	db_close(h);
}
END_TEST

START_TEST (test_store)
{
	DBHANDLE h = NULL;
	int rc = -1;

	h = db_open(DB_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	fail_unless(NULL != h);

	rc = db_store(h, "myFirstData", "some data", DB_INSERT);
	fail_unless(0 == rc);

	/*rc = db_store(h, "myFirstData", "other data", DB_REPLACE);
	fail_unless(0 == rc);*/

	db_close(h);
}
END_TEST

START_TEST (test_delete)
{
	DBHANDLE h = NULL;
	int rc = -1;

	h = db_open(DB_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	fail_unless(NULL != h);

	db_close(h);
}
END_TEST

Suite* test_suite(void)
{
        Suite *s = suite_create("db_suite");

        /* Core test case */
        TCase *tc_core = tcase_create("Core");
        tcase_add_checked_fixture(tc_core, setup, teardown);
        tcase_add_test(tc_core, test_open);
        tcase_add_test(tc_core, test_store);
        //tcase_add_test(tc_core, test_delete);
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
