#include "Date.h"
#include <sstream>
#include <ctime>
#include <cstdlib>

typedef std::pair<unsigned int, std::string> NoStrPair;

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
	daysSinceBaseDate_ = dmy2dsbd(d, m, y);
}

int dmy2dsbd(unsigned int d, unsigned int m, unsigned int y)
{
	if (d < 1 || d > monthDays(m, y) || m < 1 || m > 12 || y < baseYear_)
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
	getMDY(d, m, y);
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
	unsigned int tmp = daysSinceBaseDate_;
	while((tmp -= daysYear(++y)) >= 0);
	tmp += daysYear(++y);
	m = 0;
	while ((tmp -= daysMonth(++m)) >= 0);
	d = tmp+daysMonth(m);
}

int Date::setToToday()
{
	time_t rawtime;
	tm *timeinfo;

	time(&rawtime);
	if (NULL == (timeinfo = localtime(&rawtime)))
	{
		return EXIT_FAILURE;
	}

	daysSinceBaseDate_ = dmy2dsbd(timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year);
	return EXIT_SUCCESS;
}

std::string Date::getWeekDay()
{
	unsigned int d, m, y;
	getDMY(d, m, y);
	return weekDay_[(d-1)%7];
}

static bool Date::leapYear(unsigned int year)
{
	bool isLeap = false;
	if (0 == year%400)
	{
		isLeap = true;
	} else if (0 == year%4 && 0 != year%100)
	{
		isLeap = true;
	}
	return isleap;
}

static std::string Date::monthName(unsigned int month)
{
	if (month < 1 || month > 12)
	{
		return std::string("unknown month");
	}
	return monthName_[month-1];
}

static unsigned int Date::yearDays(unsigned int year)
{
	return leapYear(year)?DAYS_LEAP_YEAR:DAYS_NOT_LEAP_YEAR;
}

static unsigned int Date::monthDays(unsigned int month, unsigned int year)
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
