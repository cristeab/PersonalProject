#ifndef EMPLOYER_H
#define EMPLOYER_H

#include <string>
#include "Person.h"
#include "Position.h"
#include <vector>

/**
 * class Employer
 *
 */

class Employer
{
public:
	Employer() :
		name_(""), market_("")
	{
	}

	virtual ~Employer()
	{}

	bool hire(Person &newH, Position &pos)
	{
		newH.setPosition(this, &pos);
		persList_.push_back(&newH);
		pos.setEmployer(this);
		return true;
	}

	Employer(const std::string &name, const std::string &market) :
		name_(name), market_(market)
	{
	}

	std::string toString() const
	{
		std::string out;
		out = "Name: " + name_;
		out += "\nMarket: " + market_;
		return out;
	}

	std::string getName() const
	{
		return name_;
	}

	int getNbEmployees() const
	{
		return persList_.size();
	}
private:
	std::string name_;
	std::string market_;
	std::vector<Person*> persList_;
};

#endif // EMPLOYER_H
