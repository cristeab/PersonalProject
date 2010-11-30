/*
 * Person.cpp
 *
 *  Created on: Nov 30, 2010
 *      Author: bogdan
 */

#include "Person.h"
#include "Position.h"
#include "Employer.h"

std::string Person::toString() const
{
	std::string out;
	out = "Name: " + name_;
	if (employed_)
	{
		out += "\nEmployed: yes";
	} else
	{
		out += "\nEmployed: no";
	}
	out += "\nEmployer: " + employer_->toString();
	out += "\nPosition: " + position_->toString();
	return out;
}

std::string Person::getPosition()
{
	return employed_?position_->getName():std::string("no money");
}

std::string Person::getEmployer()
{
	return employed_?employer_->getName():std::string("loser");
}
