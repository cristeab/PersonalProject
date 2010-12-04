/*
 * test_hondurota.cpp
 *
 *  Created on: Dec 4, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include "Hondurota.h"

TEST (test_case, your_test)
{
	Hondurota car(0, 100.0/6.0);
	EXPECT_EQ(car.addFuel(30), 30);
	EXPECT_EQ(car.addFuel(59), 59);
	EXPECT_EQ(car.addFuel(60), car.getTankCapacity());
	EXPECT_EQ(car.addFuel(61), car.getTankCapacity());
	EXPECT_EQ(car.addFuel(0), car.getTankCapacity());
	EXPECT_EQ(car.addFuel(-30), -1);

	EXPECT_EQ(car.drive(-20, 10), car.getFuel());
	EXPECT_EQ(car.drive(20, 0), car.getFuel());

	EXPECT_EQ(car.getTankCapacity(), car.getFuel());
	EXPECT_EQ(car.drive(100, 300), car.getTankCapacity()/2);
	EXPECT_EQ(car.drive(100, 300), 0);
	EXPECT_EQ(car.getOdometer(), 1000);
	EXPECT_EQ(car.drive(10, 5), 0);
	EXPECT_EQ(car.getOdometer(), 1000);

	EXPECT_EQ(car.addFuel(6), 6);
	EXPECT_EQ(car.drive(10, 6), 6*99.0/100);
	EXPECT_EQ(car.getOdometer(), 1001);
	EXPECT_EQ(car.getSpeed(), 10);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
