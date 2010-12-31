/*
 * state_pattern.cpp
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#include <gtest/gtest.h>
#include <iostream>

#include "TCPConnection.h"
#include "TCPState.h"

TEST (test_case, your_test)
{
	TCPConnection conn;
	conn.ActiveOpen();
	conn.Transmit(NULL);
	conn.Close();
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
