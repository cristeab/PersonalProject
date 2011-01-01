/*
 * template_method_pattern.cpp
 *
 *  Created on: Jan 1, 2011
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <iostream>

class View
{
public:
	void Display()
	{
		SetFocus();
		DoDisplay();
		ResetFocus();
	}
protected:
	virtual void DoDisplay()//this is a hook, may be overwritten
	{
		std::cout << "DoDisplay from View" << std::endl;
	}
private:
	void SetFocus()
	{
		std::cout << "set focus" << std::endl;
	}
	void ResetFocus()
	{
		std::cout << "reset focus" << std::endl;
	}
};

class View1 : public View
{
protected:
	virtual void DoDisplay()//this is a hook, may be overwritten
	{
		std::cout << "DoDisplay from View1" << std::endl;
	}
};

class View2 : public View
{
protected:
	virtual void DoDisplay()//this is a hook, may be overwritten
	{
		std::cout << "DoDisplay from View2" << std::endl;
	}
};

TEST (test_case, your_test)
{
	View *view = new View2();

	view->Display();

	View *viewb = new View1();
	viewb->Display();

	View *viewc = new View();
	viewc->Display();

	delete view;
	delete viewb;
	delete viewc;
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
