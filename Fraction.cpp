/*
 * Fraction.cpp
 *
 *  Created on: Nov 28, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <string>
#include <sstream>

class Fraction
{
public:
	Fraction(int nominator = 1, int denominator = 1) :
		nominator_(nominator), denominator_(denominator)
	{
		simplify();
	}
	void set(int nominator, int denominator)
	{
		nominator_ = nominator;
		denominator_ = denominator;
		simplify();
	}

	double toDouble()
	{
		return double(nominator_) / denominator_;
	}

	std::string toString()
	{
		if ((0 == nominator_) && (0 == denominator_))
		{
			return std::string("NaN");
		}
		if (0 == nominator_)
		{
			return std::string(0);
		}
		if (0 == denominator_)
		{
			return (nominator_>0)?std::string("+inf"):std::string("-inf");
		}
		if ((1 == nominator_) && (1 == denominator_))
		{
			return std::string("1");
		}
		std::stringstream out;
		if (1 == denominator_)
		{
			out << nominator_;
		} else
		{
			out << nominator_ << "/" << denominator_;
		}
		return out.str();
	}

	Fraction add(const Fraction& other)
	{
		int tmp_nominator = nominator_;
		int tmp_denominator = denominator_;
		nominator_ = tmp_nominator * other.denominator_ + tmp_denominator
				* other.nominator_;
		denominator_ = denominator_ * other.denominator_;
		simplify();
		return *this;
	}

	Fraction substract(const Fraction& other)
	{
		int tmp_nominator = nominator_;
		int tmp_denominator = denominator_;
		nominator_ = tmp_nominator * other.denominator_ - tmp_denominator
				* other.nominator_;
		denominator_ = denominator_ * other.denominator_;
		simplify();
		return *this;
	}

	Fraction multiply(const Fraction& other)
	{
		nominator_ *= other.nominator_;
		denominator_ *= other.denominator_;
		simplify();
		return *this;
	}

	Fraction divide(const Fraction& other)
	{
		nominator_ *= other.denominator_;
		denominator_ *= other.nominator_;
		simplify();
		return *this;
	}

private:
	void simplify()
	{
		if ((0 == nominator_) || (0 == denominator_) || (1 == nominator_) || (1 == denominator_))
		{
			return;
		}
		int d = gcd(nominator_, denominator_);
		while (1 != d)
		{
			nominator_ /= d;
			denominator_ /= d;
			d = gcd(nominator_, denominator_);
		}
	}

	int gcd(int a, int b)
	{
	    while( 1 )
	    {
	        a = a % b;
			if( a == 0 )
				return b;
			b = b % a;

	        if( b == 0 )
				return a;
	    }
	}

	int nominator_;
	int denominator_;
};

TEST (test_fraction, fraction_test)
{
	Fraction frac;
	EXPECT_EQ(frac.toDouble(), 1);
	EXPECT_STREQ(frac.toString().c_str(), "1");

	frac.set(1, 10);
	EXPECT_EQ(frac.toDouble(), 0.1);
	EXPECT_STREQ(frac.toString().c_str(), "1/10");

	frac.set(10, 1);
	EXPECT_EQ(frac.toDouble(), 10);
	EXPECT_STREQ(frac.toString().c_str(), "10");

	frac.set(4, 5);
	Fraction frac2(1, 5);
	EXPECT_EQ(frac.add(frac2).toDouble(), 1);

	frac.set(5, 12);
	frac2.set(17, 12);
	EXPECT_EQ(frac.substract(frac2).toString(), "-1");

	frac.set(5, 12);
	frac2.set(3, 5);
	EXPECT_EQ(frac.multiply(frac2).toDouble(), 0.25);
	frac.set(5, 12);
	EXPECT_STREQ(frac.multiply(frac2).toString().c_str(), "1/4");

	frac.set(5, 12);
	frac2.set(10, 4);
	EXPECT_EQ(frac.divide(frac2).toString(),"1/6");
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
