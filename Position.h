#ifndef POSITION_H
#define POSITION_H

#include <string>

/**
 * class Position
 *
 */

#include <string>
#include <vector>

class Position
{
public:
	Position() :
		name_(""), description_("")
	{}
	virtual ~Position()
	{}
	Position(const std::string &name, const std::string &description) :
		name_(name), description_(description)
	{
	}
	std::string toString() const
	{
		std::string out;
		out = "Name: " + name_;
		out += "\nDescription: " + description_;
		return out;
	}
	std::string getName() const
	{
		return name_;
	}
	void setEmployer(Employer *emp)
	{
		empList_.push_back(emp);
	}
private:
	std::string name_;
	std::string description_;
	std::vector<Employer*> empList_;
};

#endif // POSITION_H
