#include "Date.h"
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <error.h>

long Date::baseYear_ = 1000;
std::string Date::monthName_[12];
std::string Date::weekDay_[7];

void Date::init()
{
	monthName_[0] = "January";
	monthName_[1] = "February";
	monthName_[2] = "March";
	monthName_[3] = "April";
	monthName_[4] = "May";
	monthName_[5] = "June";
	monthName_[6] = "July";
	monthName_[7] = "August";
	monthName_[8] = "September";
	monthName_[9] = "October";
	monthName_[10] = "November";
	monthName_[11] = "December";

	weekDay_[0] = "Monday";
	weekDay_[1] =  "Tuesday";
	weekDay_[2] = "Wednesday";
	weekDay_[3] = "Thursday";
	weekDay_[4] = "Friday";
	weekDay_[5] = "Saturday";
	weekDay_[6] = "Sunday";
}

Date::Date(unsigned int d, unsigned int m, unsigned int y)
{
	init();
	daysSinceBaseDate_ = dmy2dsbd(d-1, m, y);
}

int Date::dmy2dsbd(unsigned int d, unsigned int m, unsigned int y)
{
	if (d < 1 || d > monthDays(m, y) || m < 1 || m > 12 || (y < baseYear_))
	{
		return 0;
	}

	int daysSinceBaseDate = 0;
	unsigned int n;
	for (n = baseYear_; n < y; ++n)
	{
		daysSinceBaseDate += yearDays(n);
	}
	for (n = 1; n < m; ++n)
	{
		daysSinceBaseDate += monthDays(n, y);
	}
	daysSinceBaseDate += d;
	return daysSinceBaseDate;
}

std::string Date::toString(bool brief)
{
	unsigned int d, m, y;
	getDMY(d, m, y);
	std::stringstream out;
	if (brief)
	{
		out << d << '/' << m;
	} else
	{
		out << d << '/' << m << '/' << y;
	}
	return out.str();
}

void Date::getDMY(unsigned int &d, unsigned int &m, unsigned int &y)
{
	y = baseYear_-1;
	long tmp = daysSinceBaseDate_;
	while((tmp -= yearDays(++y)) >= 0);
	tmp += yearDays(y);
	m = 0;
	while ((tmp -= monthDays(++m, y)) >= 0);
	d = tmp+monthDays(m, y)+1;
}

int Date::setToToday()
{
	time_t rawtime;
	tm *timeinfo = NULL;

	if ((time_t)-1 == time(&rawtime))
	{
		return EXIT_FAILURE;
	}
	if (NULL == (timeinfo = localtime(&rawtime)))
	{
		return EXIT_FAILURE;
	}

	daysSinceBaseDate_ = dmy2dsbd(timeinfo->tm_mday, timeinfo->tm_mon+1, 1900+timeinfo->tm_year);
	return EXIT_SUCCESS;
}

std::string Date::getWeekDay()
{
	unsigned int d, m, y;
	getDMY(d, m, y);

	//algorithm for calculating the day of the week
	unsigned int century = 2*(3-(y/100)%4);
	unsigned int year = y%100;
	year += year/4;
	static const unsigned int month_table_not_leap[] = {0, 3 ,3 , 6, 1, 4, 6, 2, 5, 0, 3, 5};
	static const unsigned int month_table_leap[] = {6, 2 ,3 , 6, 1, 4, 6, 2, 5, 0, 3, 5};
	unsigned int month = leapYear(y)?month_table_leap[m-1]:month_table_not_leap[m-1];
	unsigned int day = d+century+year+month;

	return weekDay_[(day+5)%7];
}

bool Date::leapYear(unsigned int year)
{
	bool isLeap = false;
	if (0 == year%400)
	{
		isLeap = true;
	} else if (0 == year%4 && 0 != year%100)
	{
		isLeap = true;
	}
	return isLeap;
}

std::string Date::monthName(unsigned int month)
{
	if (month < 1 || month > 12)
	{
		return std::string("unknown month");
	}
	return monthName_[month-1];
}

unsigned int Date::yearDays(unsigned int year)
{
	return leapYear(year)?DAYS_LEAP_YEAR:DAYS_NOT_LEAP_YEAR;
}

unsigned int Date::monthDays(unsigned int month, unsigned int year)
{
	if (month < 1 || month > 12)
	{
		return 0;
	}

	unsigned int days = 0;
	std::string currentMonth = monthName(month);
	if (currentMonth == monthName(2))//check if this is February
	{
		days = leapYear(year)?FEB_LEAP:FEB_NOT_LEAP;
	} else if (currentMonth == monthName(4) || currentMonth == monthName(6) ||
			currentMonth == monthName(9) || currentMonth == monthName(11)) //30 days months
	{
		days = 30;
	} else
	{
		days = 31;
	}
	return days;
}
