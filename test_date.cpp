/*
 * test_date.cpp
 *
 *  Created on: Dec 15, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include "Date.h"

TEST (date_test_case, itf_test)
{
	Date date;
	EXPECT_STREQ(date.toString().c_str(), "01/01/1000");
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
