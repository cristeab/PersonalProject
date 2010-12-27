/*
 * mediator.cpp
 *
 *  Created on: Dec 27, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <deque>
#include <string>

class Widget;

//mediator interface
class DialogDirector
{
public:
	virtual ~DialogDirector()
	{}
	virtual void ShowDialog()
	{}
	virtual void WidgetChanged(Widget *w) = 0;
protected:
	DialogDirector()
	{}
	virtual void CreateWidgets() = 0;
};

class MouseEvent
{};

//widget interface
class Widget
{
public:
	Widget(DialogDirector* dd) :
		director_(dd)
	{}
	virtual void Changed()
	{
		director_->WidgetChanged(this);
	}
	virtual void HandleMouse(MouseEvent &event)
	{}
	virtual ~Widget()
	{}
private:
	DialogDirector *director_;
};

class ListBox : public Widget
{
public:
	ListBox(DialogDirector *dd) :
		Widget(dd), currentItem_(0)
	{
	}
	virtual const char* GetSelection()
	{
		if (!list_.empty())
		{
			return list_[currentItem_];
		}
		return "";
	}
	virtual void SetList(std::deque<char*> *listItems)
	{
		for (size_t n = 0; n < listItems->size(); ++n)
		{
			list_.push_back(strdup(listItems->at(n)));
		}
	}
	virtual void HandleMouse(MouseEvent &event)
	{
		currentItem_ = (currentItem_+1)%list_.size();
		Widget::Changed();
	}
	~ListBox()
	{
		for (size_t n = 0; n < list_.size(); ++n)
		{
			if (NULL != list_.at(n))
			{
				delete list_.at(n);
				list_.at(n) = NULL;
			}
		}
	}
private:
	int currentItem_;
	std::deque<char*> list_;
};

class EntryField : public Widget
{
public:
	EntryField(DialogDirector *dd):
		Widget(dd), text_(NULL)
	{}
	virtual void SetText(const char *text)
	{
		if (NULL != text_)
		{
			delete text_;
		}
		text_ = strdup(text);
	}
	virtual const char* GetText()
	{
		return text_;
	}
	virtual void HandleMouse(MouseEvent &event)
	{
		Widget::Changed();
	}
	~EntryField()
	{
		if (NULL != text_)
		{
			delete text_;
		}
	}
private:
	char *text_;
};

class Button : public Widget
{
public:
	Button(DialogDirector *dd) :
		Widget(dd), text_(NULL)
	{}
	virtual void SetText(const char *text)
	{
		if (NULL != text_)
		{
			delete text_;
		}
		text_ = strdup(text);
		Widget::Changed();
	}
	virtual void HandleMouse(MouseEvent &event)
	{
		Widget::Changed();
	}
private:
	char *text_;
};

//mediator implementation
class FontDialogDirector : public DialogDirector
{
public:
	FontDialogDirector() :
		buttonOK_(NULL), buttonCancel_(NULL), fontList_(NULL), fontName_(NULL)
	{}
	virtual ~FontDialogDirector()
	{
		if (NULL != buttonOK_)
		{
			delete buttonOK_;
		}
		if (NULL != buttonCancel_)
		{
			delete buttonCancel_;
		}
		if (NULL != fontList_)
		{
			delete fontList_;
		}
		if (NULL != fontName_)
		{
			delete fontName_;
		}
	}
	virtual void WidgetChanged(Widget *w)
	{
		if (NULL == w)
		{
			return;
		}
		if (buttonOK_ == w)
		{
			std::cout << "button OK" << std::endl;
		}
		else if (buttonCancel_ == w)
		{
			std::cout << "button Cancel" << std::endl;
		} else if (fontList_ == w)
		{
			std::cout << "font list" << std::endl;
			std::cout << "selected font is " << fontList_->GetSelection() << std::endl;
		} else if (fontName_ == w)
		{
			std::cout << "font name " << fontName_->GetText() << std::endl;
		} else
		{
			std::cout << "unknown widget" << std::endl;
		}
	}
	virtual void CreateWidgets()
	{
		buttonOK_ = new Button(this);
		buttonCancel_ = new Button(this);
		fontList_ = new ListBox(this);
		fontName_ = new EntryField(this);

		//fill the list box
		std::deque<char*> *listItems = new std::deque<char*>();
		char msg1[] = "Times New Roman";
		listItems->push_back(msg1);
		char msg2[] = "Arial";
		listItems->push_back(msg2);
		char msg3[] = "Verdana";
		listItems->push_back(msg3);
		fontList_->SetList(listItems);
		delete listItems;

		//set the font name
		fontName_->SetText("Verdana");
	}
	Button* GetButtonOK()
	{
		return buttonOK_;
	}
	Button* GetButtonCancel()
	{
		return buttonCancel_;
	}
	ListBox* GetFontList()
	{
		return fontList_;
	}
	EntryField* GetFontName()
	{
		return fontName_;
	}
private:
	Button *buttonOK_;
	Button *buttonCancel_;
	ListBox *fontList_;
	EntryField *fontName_;
};

TEST (test_case, your_test)
{
	FontDialogDirector dlgDir;
	dlgDir.CreateWidgets();

	Button *btn = dlgDir.GetButtonOK();
	EXPECT_TRUE(NULL != btn);
	btn->SetText("OK");

	ListBox *fontList = dlgDir.GetFontList();
	EXPECT_TRUE(NULL != fontList);
	MouseEvent evt;
	fontList->HandleMouse(evt);

	EntryField *fontName = dlgDir.GetFontName();
	EXPECT_TRUE(NULL != fontName);
	fontName->HandleMouse(evt);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
