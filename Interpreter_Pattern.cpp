/*
 * Interpreter_Pattern.cpp
 *
 *  Created on: Dec 24, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include "BooleanExp.h"
#include "VariableExp.h"
#include "Context.h"

class AndExp : public BooleanExp
{
public:
	AndExp(BooleanExp *exp1, BooleanExp *exp2) :
		exp1_(exp1), exp2_(exp2)
	{
	}
	virtual bool Evaluate(Context &context)
	{
		return exp1_->Evaluate(context) && exp2_->Evaluate(context);
	}
	virtual BooleanExp* Replace(const char *expr_str, BooleanExp &exp)
	{
		return new AndExp(exp1_->Replace(expr_str, exp), exp2_->Replace(expr_str, exp));
	}
	virtual BooleanExp* Copy() const
	{
		return new AndExp(exp1_->Copy(), exp2_->Copy());
	}
private:
	BooleanExp *exp1_;
	BooleanExp *exp2_;
};

class OrExp : public BooleanExp
{
public:
	OrExp(BooleanExp *exp1, BooleanExp *exp2) :
		exp1_(exp1), exp2_(exp2)
	{
	}
	virtual bool Evaluate(Context &context)
	{
		return exp1_->Evaluate(context) || exp2_->Evaluate(context);
	}
	virtual BooleanExp* Replace(const char *expr_str, BooleanExp &exp)
	{
		return new OrExp(exp1_->Replace(expr_str, exp), exp2_->Replace(expr_str, exp));
	}
	virtual BooleanExp* Copy() const
	{
		return new OrExp(exp1_->Copy(), exp2_->Copy());
	}
private:
	BooleanExp *exp1_;
	BooleanExp *exp2_;
};

class NotExp : public BooleanExp
{
public:
	NotExp(BooleanExp *exp) :
		exp_(exp)
	{
	}
	virtual bool Evaluate(Context &context)
	{
		return !exp_->Evaluate(context);
	}
	virtual BooleanExp* Replace(const char *expr_str, BooleanExp &exp)
	{
		return new NotExp(exp_->Replace(expr_str, exp));
	}
	virtual BooleanExp* Copy() const
	{
		return new NotExp(exp_->Copy());
	}
private:
	BooleanExp *exp_;
};

class ConstExp : public BooleanExp
{
public:
	ConstExp(bool value) :
		value_(value)
	{
	}
	virtual ~ConstExp()
	{
	}
	virtual bool Evaluate(Context &context)
	{
		return value_;
	}
	virtual BooleanExp* Replace(const char */*expr_str*/, BooleanExp &/*exp*/)
	{
		return new ConstExp(value_);
	}
	virtual BooleanExp* Copy() const
	{
		return new ConstExp(value_);
	}
private:
	bool value_;
};

TEST (interpreter_case, notexp_test)
{
	ConstExp const_exp(true);
	BooleanExp *exp = new NotExp(&const_exp);
	Context context;
	ASSERT_FALSE(exp->Evaluate(context));

	const_exp = false;
	exp = new NotExp(&const_exp);
	ASSERT_TRUE(exp->Evaluate(context));

	VariableExp x("X");
	if (NULL != exp)
	{
		delete exp;
	}
	exp = new NotExp(&x);

	context.Assign(&x, true);
	ASSERT_FALSE(exp->Evaluate(context));

	context.Assign(&x, false);
	ASSERT_TRUE(exp->Evaluate(context));

	delete exp;
}

TEST (interpreter_case, orexp_test)
{
	ConstExp const_exp1(true);
	ConstExp const_exp2(true);
	BooleanExp *exp = new OrExp(&const_exp1, &const_exp2);
	Context context;
	ASSERT_TRUE(exp->Evaluate(context));

	const_exp2 = false;
	ASSERT_TRUE(exp->Evaluate(context));

	const_exp1 = false;
	ASSERT_FALSE(exp->Evaluate(context));

	VariableExp x("X");
	VariableExp y("Y");
	if (NULL != exp)
	{
		delete exp;
	}
	exp = new OrExp(&x, &y);

	context.Assign(&x, true);
	context.Assign(&y, true);
	ASSERT_TRUE(exp->Evaluate(context));

	context.Assign(&x, true);
	context.Assign(&y, false);
	ASSERT_TRUE(exp->Evaluate(context));

	context.Assign(&x, false);
	context.Assign(&y, true);
	ASSERT_TRUE(exp->Evaluate(context));

	context.Assign(&x, false);
	context.Assign(&y, false);
	ASSERT_FALSE(exp->Evaluate(context));

	if (NULL != exp)
	{
		delete exp;
	}
	exp = new OrExp(&const_exp1, &y);
	ASSERT_FALSE(exp->Evaluate(context));

	const_exp1 = true;
	ASSERT_TRUE(exp->Evaluate(context));

	delete exp;
}

TEST (interpreter_case, andexp_test)
{
	ConstExp const_exp1(true);
	ConstExp const_exp2(true);
	BooleanExp *exp = new AndExp(&const_exp1, &const_exp2);
	Context context;
	ASSERT_TRUE(exp->Evaluate(context));

	const_exp2 = false;
	ASSERT_FALSE(exp->Evaluate(context));

	const_exp1 = false;
	ASSERT_FALSE(exp->Evaluate(context));

	VariableExp x("X");
	VariableExp y("Y");
	if (NULL != exp)
	{
		delete exp;
	}
	exp = new OrExp(&x, &y);

	context.Assign(&x, false);
	context.Assign(&y, false);
	ASSERT_FALSE(exp->Evaluate(context));

	context.Assign(&x, false);
	context.Assign(&y, true);
	ASSERT_TRUE(exp->Evaluate(context));

	context.Assign(&x, true);
	context.Assign(&y, false);
	ASSERT_TRUE(exp->Evaluate(context));

	context.Assign(&x, true);
	context.Assign(&y, true);
	ASSERT_TRUE(exp->Evaluate(context));

	delete exp;
}

//evaluate the expression: (true and x) or (y and (not x))
TEST (interpreter_case, expression_test)
{
	BooleanExp *expression;
	Context context;

	VariableExp *x = new VariableExp("X");
	VariableExp *y = new VariableExp("Y");

	expression = new OrExp(new AndExp(new ConstExp(true), x), new AndExp(y, new NotExp(x)));

	context.Assign(x, false);
	context.Assign(y, true);

	ASSERT_TRUE(expression->Evaluate(context));
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
