/*
 * test_date.cpp
 *
 *  Created on: Dec 15, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <sstream>
#include <ctime>
#include <string>
#include "Date.h"

TEST (date_test_case, static_methods_test)
{
	Date date;

	EXPECT_FALSE(date.leapYear(2010));
	EXPECT_TRUE(date.leapYear(2012));

	EXPECT_STREQ(date.monthName(1).c_str(), "January");
	EXPECT_STREQ(date.monthName(8).c_str(), "August");
	EXPECT_STREQ(date.monthName(11).c_str(), "November");

	EXPECT_EQ(date.yearDays(2010), 365);
	EXPECT_EQ(date.yearDays(2012), 366);

	EXPECT_EQ(date.monthDays(2, 2010), 28);
	EXPECT_EQ(date.monthDays(2, 2012), 29);
	EXPECT_EQ(date.monthDays(11, 2010), 30);
	EXPECT_EQ(date.monthDays(8, 2010), 31);
}

TEST (date_test_case, operation_methods_on_date_test)
{
	Date date[2];

	EXPECT_TRUE(date[0].equals(date[1]));

	EXPECT_TRUE(date[0].addDays(0).equals(date[1]));
	EXPECT_FALSE(date[0].addDays(1).lessThan(date[1]));
	EXPECT_TRUE(date[0].addDays(-2).lessThan(date[1]));
	EXPECT_TRUE(date[0].addDays(1).equals(date[1]));

	EXPECT_EQ(date[0].daysAfter(date[1]), 0);
	EXPECT_EQ(date[0].daysAfter(date[1].addDays(1)), -1);
	EXPECT_EQ(date[0].daysAfter(date[1].addDays(-2)), 1);
	EXPECT_EQ(date[0].daysAfter(date[1].addDays(1)), 0);
}

TEST (date_test_case, current_date_methods_test)
{
	Date date;

	EXPECT_EQ(date.setToToday(), EXIT_SUCCESS);

	//get current time
	time_t rawtime;
	if ((time_t)-1 == time(&rawtime))
	{
		ASSERT_TRUE(false);
	}
	tm *timeinfo = NULL;
	if (NULL == (timeinfo = localtime(&rawtime)))
	{
		ASSERT_TRUE(false);
	}
	const char *weekDay[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

	EXPECT_EQ(date.getWeekDay(), weekDay[timeinfo->tm_wday%7]);
	date.addDays(2);
	EXPECT_EQ(date.getWeekDay(), weekDay[(timeinfo->tm_wday+2)%7]);
	date.addDays(7);
	EXPECT_EQ(date.getWeekDay(), weekDay[(timeinfo->tm_wday+9)%7]);

	//convert current day to expected format
	std::stringstream out;
	out << timeinfo->tm_mday+10 << '/' << timeinfo->tm_mon+1 << '/' << timeinfo->tm_year+1900;

	EXPECT_STREQ(date.toString(false).c_str(), out.str().c_str());

	std::stringstream out2;
	out2 << timeinfo->tm_mday+10 << '/' << timeinfo->tm_mon+1;
	EXPECT_STREQ(date.toString(true).c_str(), out2.str().c_str());
}

TEST (date_test_case, constructors_test)
{
	Date date;

	EXPECT_STREQ(date.toString().c_str(), "1/1/1000");

	Date date2(19, 11, 1977);
	EXPECT_STREQ(date2.toString().c_str(), "19/11/1977");
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
