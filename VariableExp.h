/*
 * VariableExp.h
 *
 *  Created on: Dec 24, 2010
 *      Author: bogdan
 */

#ifndef VARIABLEEXP_H_
#define VARIABLEEXP_H_

/*
 *
 */
#include "BooleanExp.h"

class VariableExp: public BooleanExp
{
public:
	VariableExp(const char *expr_str);
	virtual ~VariableExp();
	virtual bool Evaluate(Context &context);
	virtual BooleanExp* Replace(const char *name, BooleanExp &exp);
	virtual BooleanExp* Copy() const;
	char* GetName() const;
private:
	char *name_;
};

#endif /* VARIABLEEXP_H_ */
