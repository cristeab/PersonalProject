/*
 * IteratorPattern.cpp
 *
 * Implements both external and internal iterators
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
			throw IteratorOutOfBounds();
		}
		return pList_->Get(current_);
	}
private:
	const List<Item> *pList_;
	long current_;
};

template <class Item>
class ReverseListIterator : public Iterator<Item>
{
public:
	ReverseListIterator(const List<Item> *pList) :
		pList_(pList), current_(0)
	{
	}
	virtual void First()
	{
		current_ = pList_->Count()-1;
	}
	virtual void Next()
	{
		--current_;
	}
	virtual bool IsDone() const
	{
		return current_ < 0;
	}
	virtual Item& CurrentItem() const
	{
		if (IsDone())
		{
			throw IteratorOutOfBounds();
		}
		return pList_->Get(current_);
	}
private:
	const List<Item> *pList_;
	long current_;
};

class IteratorTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		for (int n = 0; n < 10; ++n)
		{
			fixture_.Set(n, n+100);
		}
	}
	virtual void TearDown()
	{}
	List<int> fixture_;
};

//this is a proxy
template <class Item>
class IteratorPtr
{
public:
	IteratorPtr(Iterator<Item> *ipIt) :
		pIt_(ipIt)
	{
	}
	Iterator<Item>* operator->()
	{
		return pIt_;
	}
	Iterator<Item>& operator*()
	{
		return *pIt_;
	}
	~IteratorPtr()
	{
		delete pIt_;
	}
private:
	IteratorPtr(const IteratorPtr<Item>&);
	IteratorPtr& operator=(const IteratorPtr<Item>&);
	Iterator<Item> *pIt_;
};

//internal iterator
template<class Item>
class ListTraverser
{
public:
	ListTraverser(Iterator<Item> *ipIterator) :
		pIt_(ipIterator)
	{
	}
	bool Traverse()
	{
		bool result = false;
		for (pIt_->First(); !pIt_->IsDone(); pIt_->Next())
		{
			result = ProcessItem(pIt_->CurrentItem());
			if (false == result)
			{
				break;
			}
		}
		return result;
	}
protected:
	virtual bool ProcessItem(const Item &item) = 0;
private:
	Iterator<Item> *pIt_;
};

template<class Item>
class GetNbItems : public ListTraverser<Item>
{
public:
	GetNbItems(Iterator<Item> *ipIterator) :
		ListTraverser<Item>(ipIterator), count_(0)
	{
	}
	long GetCount()
	{
		return count_;
	}
protected:
	virtual bool ProcessItem(const Item & item)
	{
		++count_;
		return true;
	}
private:
	long count_;
};

TEST_F (IteratorTest, iterators_test)
{
	//iterate through the entire list
	IteratorPtr<int> pIt(new ListIterator<int>(&fixture_));
	int n = 0;
	for (pIt->First(), n = 0; !pIt->IsDone(); pIt->Next(), ++n)
	{
		EXPECT_EQ(pIt->CurrentItem(), n+100);
	}
	IteratorPtr<int> pIt2(new ReverseListIterator<int>(&fixture_));
	for (pIt2->First(), n = 9; !pIt2->IsDone(); pIt2->Next(), --n)
	{
		EXPECT_EQ(pIt2->CurrentItem(), n+100);
	}
}

TEST_F (IteratorTest, throw_exception)
{
	IteratorPtr<int> pIt(new ListIterator<int>(&fixture_));
	pIt->First();
	for (int n = 0; n < 11; ++n)
	{
		pIt->Next();
	}
	try
	{
		pIt->CurrentItem();
	}
	catch (IteratorOutOfBounds e)
	{
		return;
	}
	catch (...)
	{
		//must fail
		ASSERT_EQ(0, 1);
	}
	ASSERT_EQ(0, 1);
}

TEST_F (IteratorTest, test_internal_iterator)
{
	IteratorPtr<int> pIt(new ListIterator<int>(&fixture_));
	GetNbItems<int> nbItems(pIt.operator->());
	EXPECT_EQ(nbItems.Traverse(), true);
	EXPECT_EQ(nbItems.GetCount(), 10);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
