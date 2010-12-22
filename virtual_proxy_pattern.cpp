/*
 * virtual_proxy_pattern.cpp
 *
 *  Created on: Dec 22, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <fstream>
#include <cstring>

class Point
{
	friend bool operator==(const Point &p1, const Point &p2);
public:
	Point() :
		x_(0), y_(0)
	{}
	Point(double x, double y) :
		x_(x), y_(y)
	{}
private:
	double x_;
	double y_;
};

bool operator==(const Point &p1, const Point &p2)
{
	return ((p1.x_ == p2.x_) && (p1.y_ == p2.y_));
}

class Event;

class Graphic
{
public:
	virtual ~Graphic()
	{}
	virtual void Draw(const Point &at) = 0;
	virtual void HandleMouse(Event &event) = 0;
	virtual const Point& GetExtent() = 0;
	virtual void Load(std::ifstream &from) = 0;
	virtual void Save(std::ofstream &to) = 0;
protected:
	Graphic()
	{}
};

class Image : public Graphic
{
public:
	Image(const char *file) :
		point_(), line_()
	{
		fileName_ = strdup(file);
	}
	virtual ~Image()
	{
		delete fileName_;
	}
	virtual void Draw(const Point &at)
	{
		point_ = at;
	}
	virtual void HandleMouse(Event &event)
	{

	}
	virtual const Point& GetExtent()
	{
		return point_;
	}
	virtual void Load(std::ifstream &from)
	{
		char line[MAX_LINE_LEN];
		from.getline(line, MAX_LINE_LEN);
		line_ = line;
	}
	virtual void Save(std::ofstream &to)
	{
		to << line_;
	}
private:
	enum {MAX_LINE_LEN = 1024};
	char *fileName_;
	Point point_;
	std::string line_;
};

class ImageProxy : public Graphic
{
public:
	ImageProxy(const char *imageFile) :
		image_(NULL)
	{
		imageFile_ = strdup(imageFile);
	}
	virtual ~ImageProxy()
	{
		if (image_)
		{
			delete image_;
			image_ = NULL;
		}
		if (imageFile_)
		{
			delete imageFile_;
			imageFile_ = NULL;
		}
	}
	virtual void Draw(const Point &at)
	{
		GetImage()->Draw(at);
	}
	virtual void HandleMouse(Event &event)
	{
		GetImage()->HandleMouse(event);
	}
	virtual const Point& GetExtent()
	{
		return GetImage()->GetExtent();
	}
	virtual void Load(std::ifstream &from)
	{
		GetImage()->Load(from);
	}
	virtual void Save(std::ofstream &to)
	{
		GetImage()->Save(to);
	}
protected:
	Image* GetImage()
	{
		if (NULL == image_)
		{
			image_ = new Image(imageFile_);
		}
		return image_;
	}
private:
	Image *image_;
	char *imageFile_;
};

class TextDocument
{
public:
	TextDocument() :
		graph_(NULL)
	{}
	void Insert(Graphic *graph)
	{
		if (graph_)
		{
			delete graph_;
		}
		graph_ = graph;
	}
	Graphic* GetGraphic()
	{
		return graph_;
	}
	~TextDocument()
	{
		if (graph_)
		{
			delete graph_;
			graph_ = NULL;
		}
	}
private:
	Graphic *graph_;
};

TEST (test_case, your_test)
{
	TextDocument *text = new TextDocument();
	ASSERT_TRUE(text->GetGraphic() == NULL);
	text->Insert(new ImageProxy("virtual_proxy_pattern.cpp"));

	Graphic *graph = text->GetGraphic();//image object is not yet instantiated
	ASSERT_TRUE(graph != NULL);
	ASSERT_TRUE(graph->GetExtent() == Point(0, 0));
	Point pt(10, 5);
	graph->Draw(pt);
	Point pt2 = graph->GetExtent();
	ASSERT_TRUE(pt2 == pt);

	delete text;
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
