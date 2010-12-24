/*
 * VariableExp.cpp
 *
 *  Created on: Dec 24, 2010
 *      Author: bogdan
 */

#include "VariableExp.h"
#include "Context.h"
#include <cstring>

VariableExp::VariableExp(const char *expr_str)
{
	name_ = strdup(expr_str);
}

VariableExp::~VariableExp()
{
	if (name_)
	{
		delete name_;
	}
}

bool VariableExp::Evaluate(Context &context)
{
	return context.Lookup(name_);
}

BooleanExp* VariableExp::Replace(const char *name, BooleanExp &exp)
{
	if (0 == strncasecmp(name, name_, strlen(name_)))
	{
		return exp.Copy();
	}
	return new VariableExp(name_);
}

BooleanExp* VariableExp::Copy() const
{
	return new VariableExp(name_);
}

char* VariableExp::GetName() const
{
	return name_;
}
