/*
 * test_person.cpp
 *
 *  Created on: Nov 30, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include "Person.h"
#include "Employer.h"
#include "Position.h"

TEST (test_person, person_test)
{
	Person pers("BC");

	EXPECT_STREQ(pers.getPosition().c_str(), "no money");
	EXPECT_STREQ(pers.getEmployer().c_str(), "loser");

	Employer emp("Nivis", "IT");
	Position pos("programmer", "Linux programming");

	EXPECT_TRUE(emp.hire(pers, pos));

	EXPECT_STREQ(pers.getPosition().c_str(), "programmer");
	EXPECT_STREQ(pers.getEmployer().c_str(), "Nivis");
}

TEST (test_person, test_list_employees)
{
	Person jlp("Jean Luc Piccard");
	Person wc("Wesley Crusher");

	Employer emp1("Star Fleet Federation", "galaxy");
	Position cpt("captain", "the big boss");
	Position lieut("lieutenant", "second in command");

	emp1.hire(jlp, cpt);
	emp1.hire(wc, lieut);

	EXPECT_EQ(emp1.getNbEmployees(), 2);

	EXPECT_STREQ(jlp.getEmployer().c_str(), emp1.getName().c_str());
	EXPECT_STREQ(wc.getEmployer().c_str(), emp1.getName().c_str());
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
