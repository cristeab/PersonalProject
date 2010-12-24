/*
 * Context.cpp
 *
 *  Created on: Dec 24, 2010
 *      Author: bogdan
 */

#include <cstring>
#include "Context.h"
#include "VariableExp.h"

bool Context::Lookup(const char* name) const
{
	if (keyPair_.empty())
	{
		return false;
	}
	return keyPair_.at(std::string(name)).second;

}
void Context::Assign(VariableExp *exp, bool value)
{
	VarValue pair(exp, value);
	keyPair_[std::string(exp->GetName())] = pair;
}
