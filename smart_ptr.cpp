/*
 * smart_ptr.cpp
 *
 *  Created on: Dec 17, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>

#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/weak_ptr.hpp>

using testing::internal::scoped_ptr;

TEST (boost_utility_test_case, scoped_test)
{
	boost::scoped_ptr<int> spInt(new int);
	*spInt = 1;
	ASSERT_EQ(*spInt, 1);
	boost::scoped_array<int> spArray(new int[10]);
	for (int n = 0; n < 10; ++n)
	{
		spArray[n] = n;
	}
	EXPECT_EQ(spArray[9], 9);
}

class SomeClass
{
public:
	~SomeClass()
	{
		//std::cout << "SomeClass dtor" << std::endl;
	}
};

TEST (boost_utility_test_case, shared_test)
{
	boost::shared_ptr<int> spInt1(new int);
	*spInt1 = 23;
	EXPECT_EQ(*spInt1, 23);
	boost::shared_ptr<int> spInt2 = spInt1;
	spInt2.reset();
	EXPECT_EQ(*spInt1, 23);
	spInt1.reset();
	EXPECT_TRUE(spInt1.get() == NULL);

	boost::shared_array<SomeClass> spObj1(new SomeClass[10]);
	boost::shared_array<SomeClass> spObj2 = spObj1;
}

class ClassWithCounter;

namespace boost
{
    void intrusive_ptr_add_ref(ClassWithCounter *p);
    void intrusive_ptr_release(ClassWithCounter *p);
};

class ClassWithCounter
{
	friend void boost::intrusive_ptr_add_ref(ClassWithCounter *p);
	friend void boost::intrusive_ptr_release(ClassWithCounter *p);
public:
	ClassWithCounter() :
		counter_(0)
	{}
	int GetCounter()
	{
		return counter_;
	}
	~ClassWithCounter()
	{
		std::cout << "ClassWithCounter dtor" << std::endl;
	}
private:
	int counter_;
};

namespace boost
{
    void intrusive_ptr_add_ref(ClassWithCounter *p)
    {
    	++p->counter_;
    }
    void intrusive_ptr_release(ClassWithCounter *p)
    {
    	if (--p->counter_ <= 0)
    	{
    		delete p;
    	}
    }
};

TEST (boost_utility_test_case, intrusive_test)
{
	boost::intrusive_ptr<ClassWithCounter> spObj(new ClassWithCounter);
	boost::intrusive_ptr<ClassWithCounter> spObj2 = spObj;

	EXPECT_EQ(spObj->GetCounter(), 2);

	spObj.reset();
	EXPECT_EQ(spObj2->GetCounter(), 1);
}

TEST (boost_utility_test_case, weak_test)
{
	boost::shared_ptr<int> spInt(new int);
	*spInt = 45;
	boost::weak_ptr<int> wspInt(spInt);
	boost::shared_ptr<int> spInt2(spInt);

	EXPECT_EQ(*wspInt.lock(), 45);
	spInt.reset();

	EXPECT_EQ(*spInt2, 45);
	EXPECT_EQ(*wspInt.lock(), 45);

	spInt2.reset();
	EXPECT_TRUE(wspInt.lock().get() == NULL);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
