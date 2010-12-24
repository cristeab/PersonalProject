/*
 * Context.h
 *
 *  Created on: Dec 24, 2010
 *      Author: bogdan
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#include <cstdlib>
#include <map>
#include <utility>
#include <string>

class VariableExp;

/*
 *
 */
class Context
{
public:
	Context()
	{
	}
	bool Lookup(const char* name) const;
	void Assign(VariableExp *exp, bool value);
	~Context()
	{}
private:
	typedef std::pair<VariableExp*, bool> VarValue;
	typedef std::map<std::string, VarValue> KeyPair;
	KeyPair keyPair_;
};

#endif /* CONTEXT_H_ */
