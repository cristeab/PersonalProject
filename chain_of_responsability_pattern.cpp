/*
 * chain_of_responsability_pattern.cpp
 *
 *  Created on: Dec 23, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <iostream>

typedef int Topic;
const Topic NO_HELP_TOPIC = -1;

//interface for the chain of responsability pattern
class HelpHandler
{
public:
	HelpHandler(HelpHandler *successor = NULL, Topic topic = NO_HELP_TOPIC) :
		successor_(successor), topic_(topic)
	{}
	virtual bool HasHelp()
	{
		return (NO_HELP_TOPIC != topic_);
	}
	virtual void SetHandler(HelpHandler *successor, Topic topic)
	{
		successor_ = successor;
		topic_ = topic;
	}
	virtual void HandleHelp()
	{
		if (NULL != successor_)//forward if possible, concrete handler should decide if they handle the request
		{
			successor_->HandleHelp();
		} else
		{
			std::cout << "no help available" << std::endl;
		}
	}
private:
	HelpHandler *successor_;
	Topic topic_;
};

//interface for the widget classes
class Widget : public HelpHandler
{
protected:
	Widget(Widget *parent, Topic t = NO_HELP_TOPIC):
	HelpHandler(parent, t), parent_(parent)
	{}
private:
	Widget *parent_;
};

class Button : public Widget
{
public:
	Button(Widget *widget, Topic t = NO_HELP_TOPIC) :
		Widget(widget, t)
	{}
	virtual void HandleHelp()
	{
		if (HasHelp())
		{
			std::cout << "Help from Button class" << std::endl;
		} else
		{
			HelpHandler::HandleHelp();//forward request to a successor
		}
	}
};

class Dialog : public Widget
{
public:
	Dialog(HelpHandler *h, Topic t = NO_HELP_TOPIC) :
		Widget(NULL, t)
	{
		SetHandler(h, t);
	}
	virtual void HandleHelp()
	{
		if (HasHelp())
		{
			std::cout << "help from Dialog class" << std::endl;
		} else
		{
			HelpHandler::HandleHelp();
		}
	}
};

//last object in the chain of responsability
class Application : public HelpHandler
{
public:
	Application(Topic t) :
		HelpHandler(NULL, t)
	{}
	virtual void HandleHelp()
	{
		if (HasHelp())
		{
			std::cout << "help from Application class" << std::endl;
		}//this is the last element in the chain
	}
};

const Topic PRINT_TOPIC = 1;
const Topic PAPER_ORIENTATION_TOPIC = 2;
const Topic APPLICATION_TOPIC = 3;

TEST (test_case, your_test)
{
	Application *app = new Application(APPLICATION_TOPIC);
	Dialog *dlg = new Dialog(app, PRINT_TOPIC);
	Button *btn = new Button(dlg, PAPER_ORIENTATION_TOPIC);

	btn->HandleHelp();
	dlg->HandleHelp();
	app->HandleHelp();

	Button *btn2 = new Button(dlg, NO_HELP_TOPIC);
	btn2->HandleHelp();

	Application *app2 = new Application(NO_HELP_TOPIC);
	Dialog *dlg2 = new Dialog(app2, NO_HELP_TOPIC);
	std::cout << "getting help for dialog class" << std::endl;
	dlg2->HandleHelp();

	delete app;
	delete dlg;
	delete btn;
	delete btn2;
	delete app2;
	delete dlg2;
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
