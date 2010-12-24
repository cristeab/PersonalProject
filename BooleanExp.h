/*
 * BooleanExp.h
 *
 *  Created on: Dec 24, 2010
 *      Author: bogdan
 */

#ifndef BOOLEANEXP_H_
#define BOOLEANEXP_H_

class Context;

class BooleanExp
{
public:
	BooleanExp()
	{}
	virtual ~BooleanExp()
	{}
	virtual bool Evaluate(Context &context) = 0;
	virtual BooleanExp* Replace(const char *name, BooleanExp &exp) = 0;
	virtual BooleanExp* Copy() const = 0;
};

#endif /* BOOLEANEXP_H_ */
