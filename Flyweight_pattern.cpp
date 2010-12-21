/*
 * Flyweight_pattern.cpp
 *
 *  Created on: Dec 21, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <set>
#include <string>
#include <iostream>

class Window
{
};
class GlyphContext;

class Font
{
public:
	Font(const std::string &name) :
		fontName_(name)
	{
	}
	std::string GetFontName()
	{
		return fontName_;
	}
private:
	std::string fontName_;
};

//this is the interface to flyweight object: it includes also an Iterator and uses the composite pattern
class Glyph
{
public:
	virtual ~Glyph()
	{}
	virtual void Draw(Window *w, GlyphContext &context)
	{}
	virtual void SetFont(Font *f, GlyphContext &context)
	{}
	virtual Font* GetFont(GlyphContext &context)
	{
		return NULL;
	}
	virtual void First(GlyphContext &context)
	{}
	virtual void Next(GlyphContext &context)
	{}
	virtual bool IsDone(GlyphContext &context)
	{
		return true;
	}
	virtual Glyph* Current(GlyphContext &context)
	{
		return NULL;
	}
	virtual void Insert(Glyph *f, GlyphContext &context)
	{}
	virtual void Remove(GlyphContext &context)
	{}
protected:
	Glyph()
	{}
};

class GlyphContext
{
public:
	GlyphContext() :
		index_(0)
	{}
	virtual ~GlyphContext()
	{
		std::set<Font*>::iterator it = fonts_.begin(), it_end = fonts_.end();
		for (; it != it_end; ++it)
		{
			delete *it;
		}
		fonts_.clear();
	}
	virtual void Next(int step = 1)
	{
		index_ += step;
		index_ = index_%fonts_.size();
	}
	virtual void Insert(int quantity = 1)
	{

		for (int n = 0; n < quantity; ++n)
		{
			fonts_.insert(new Font("empty"));
		}
	}
	virtual Font* GetFont()
	{
		std::set<Font*>::iterator it = fonts_.begin(), it_end = fonts_.end();
		for (unsigned int n = 0; it != it_end; ++it, ++n)
		{
			if (n == index_)
			{
				return *it;
			}
		}
		return NULL;
	}
	virtual void SetFont(Font *f, int span = 1)
	{
		fonts_.insert(f);
	}
private:
	unsigned int index_;
	std::set<Font*> fonts_;
};

//concrete Flyweight: defines the intrinsic state
class Character : public Glyph
{
public:
	Character() :
		char_(' ')
	{
	}
	Character(char c) :
		char_(c)
	{}
	virtual void Draw(Window *w, GlyphContext &context)
	{
		std::cout << "drawing character \"" << char_ << "\" with font " << context.GetFont()->GetFontName() << std::endl;
	}
private:
	char char_;
};

class Row;
class Column;

//flyweigth factory
class GlyphFactory
{
public:
	GlyphFactory()
	{
		chars_ = new Character*[NB_CHAR_CODES];
		for (unsigned int n = 0; n < NB_CHAR_CODES; ++n)
		{
			chars_[n] = NULL;
		}
	}
	virtual ~GlyphFactory()
	{
		if (chars_)
		{
			for (unsigned int n = 0; n < NB_CHAR_CODES; ++n)
			{
					if (chars_[n])
					{
						delete chars_[n];
						chars_[n] = NULL;
					}
			}
			delete[] chars_;
			chars_ = NULL;
		}
	}
	virtual Character* CreateCharacter(char c)
	{
		int n = static_cast<int>(c);
		if (0 > n || NB_CHAR_CODES <= n)
		{
			return NULL;
		}
		if (NULL == chars_[n])
		{
			chars_[n] = new Character(c);
		}
		return chars_[n];
	}
	virtual Row* CreateRow()
	{
		return NULL;//not used
	}
	virtual Column* CreateColumn()
	{
		return NULL;//not used
	}
private:
	enum {NB_CHAR_CODES = 128};
	Character **chars_;
};

TEST (test_case, your_test)
{
	GlyphContext gc;
	Font *times12 = new Font("Times-Roman-12");
	if (NULL == times12)
	{
		ASSERT_TRUE(false);
	}
	ASSERT_STREQ(times12->GetFontName().c_str(), "Times-Roman-12");
	Font *timesItalic12 = new Font("Times-Italic-12");
	if (NULL == timesItalic12)
	{
		ASSERT_TRUE(false);
	}
	ASSERT_STREQ(timesItalic12->GetFontName().c_str(), "Times-Italic-12");
	gc.SetFont(times12, 6);
	gc.SetFont(timesItalic12, 6);

	GlyphFactory gf;
	Glyph *pGlyph = gf.CreateCharacter('a');
	Window *pW = new Window();
	pGlyph->Draw(pW, gc);
	pGlyph = gf.CreateCharacter('w');
	gc.Next();
	pGlyph->Draw(pW, gc);
	pGlyph = gf.CreateCharacter('a');
	pGlyph->Draw(pW, gc);
	delete pW;
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
