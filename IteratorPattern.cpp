/*
 * IteratorPattern.cpp
 *
 *  Created on: Nov 22, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <algorithm>

template<class Item>
class List
{
public:
	List(long size = DEFAULT_LIST_SIZE) :
		size_(size), pList_(new Item[size])
	{
	}
	long Count() const
	{
		return size_;
	}
	Item& Get(long index) const
	{
		if (0 > index)
		{
			index = size_-index%size_;
		}
		return pList_[index%size_];
	}
	void Set(long index, const Item &item)
	{
		if (index < size_)
		{
			pList_[index] = item;
		} else
		{
			long old_size = size_;
			size_ *= 2;
			Item *pTempList = new Item[size_];
			std::copy(pList_, pList_+old_size, pTempList);
			delete[] pList_;
			pList_ = pTempList;
		}
	}
	~List()
	{
		delete[] pList_;
		pList_ = NULL;
	}
private:
	enum {DEFAULT_LIST_SIZE = 10};
	long size_;
	Item *pList_;
};

template <class Item>
class Iterator
{
public:
	virtual void First() = 0;
	virtual void Next() = 0;
	virtual bool IsDone() const = 0;
	virtual Item& CurrentItem() const = 0;
};

class IteratorOutOfBounds
{
};

template <class Item>
class ListIterator : public Iterator<Item>
{
public:
	ListIterator(const List<Item> *pList) :
		pList_(pList), current_(0)
	{
	}
	virtual void First()
	{
		current_ = 0;
	}
	virtual void Next()
	{
		++current_;
	}
	virtual bool IsDone() const
	{
		return current_ >= pList_->Count();
	}
	virtual Item& CurrentItem() const
	{
		if (IsDone())
		{
			//throw IteratorOutOfBounds;
		}
		return pList_->Get(current_);
	}
private:
	const List<Item> *pList_;
	long current_;
};

TEST (test_case, your_test)
{
	//init list
	List<int> fixture;
	int n = 0;
	for (n = 0; n < 10; ++n)
	{
		fixture.Set(n, n+100);
	}

	//iterate through the entire list
	ListIterator<int> it(&fixture);
	for (it.First(), n = 0; !it.IsDone(); it.Next(), ++n)
	{
		EXPECT_EQ(it.CurrentItem(), n+100);
	}
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
