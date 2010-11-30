#ifndef PERSON_H
#define PERSON_H

#include <string>

class Position;
class Employer;

/**
 * class Person
 *
 */

class Person
{
public:
	Person() :
		name_(""), employed_(false), position_(NULL), employer_(NULL)
	{
	}

	virtual ~Person()
	{
	}

	Person(const std::string &name) :
		name_(name), employed_(false), position_(NULL), employer_(NULL)
	{
	}

	std::string toString() const;

	void setPosition(const Employer *newC, const Position *newP)
	{
		employed_ = true;
		employer_ = newC;
		position_ = newP;
	}

	std::string getPosition();

	std::string getEmployer();

private:
	std::string name_;
	bool employed_;
	const Position *position_;
	const Employer *employer_;
};

#endif // PERSON_H
