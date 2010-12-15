#ifndef DATE_H
#define DATE_H

#include <string>

// Namespace
class Date
{
public:
	Date() :
		daysSinceBaseDate_(0)
	{
		init();
	}
	//
	Date(unsigned int d, unsigned int m, unsigned int y);
	// Operations
	//
	std::string toString(bool brief = true);
	int setToToday();
	std::string getWeekDay();
	//
	bool lessThan(const Date& date)
	{
		return daysSinceBaseDate_ < date.daysSinceBaseDate_;
	}
	//
	bool equals(const Date& date)
	{
		return daysSinceBaseDate_ == date.daysSinceBaseDate_;
	}
	//
	int daysAfter(const Date& date)
	{
		return  daysSinceBaseDate_ - date.daysSinceBaseDate_;
	}
	//
	Date addDays(int days)
	{
		daysSinceBaseDate_ += days;
		return *this;
	}
	//
	static bool leapYear(unsigned int year);
	//
	static std::string monthName(unsigned int month);
	//
	static unsigned int yearDays(unsigned int year);
	//
	static unsigned int monthDays(unsigned int month, unsigned int year);

private:
	enum {DAYS_LEAP_YEAR = 366, DAYS_NOT_LEAP_YEAR = 365};
	enum {FEB_LEAP = 29, FEB_NOT_LEAP = 28};
	static void init();
	// Fields
	long daysSinceBaseDate_;
	static long baseYear_;
	static std::string monthName_[12];
	static std::string weekDay_[7];

	void getDMY(unsigned int &d, unsigned int &m, unsigned int &y);
	int dmy2dsbd(unsigned int d, unsigned int m, unsigned int y);
};

long Date::baseYear_ = 1000;
std::string Date::monthName_[12];
std::string Date::weekDay_[7];

#endif //DATE_H
