/*
 * TCPClosed.cpp
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#include "TCPClosed.h"
#include <iostream>
#include "TCPEstablished.h"

TCPClosed::TCPClosed()
{
	std::cout << "TCPClosed" << std::endl;
}

TCPState* TCPClosed::Instance()
{
	if (NULL == instance_)
	{
		instance_ = new TCPClosed();
	}
	return instance_;
}

void TCPClosed::ActiveOpen(TCPConnection *conn)
{
	std::cout << "ActiveOpen from TCPClosed" << std::endl;
	ChangeState(conn, TCPEstablished::Instance());
}

void TCPClosed::PassiveOpen(TCPConnection *conn)
{
	std::cout << "PassiveOpen from TCPClosed" << std::endl;
	ChangeState(conn, TCPEstablished::Instance());
}

TCPState* TCPClosed::instance_ = NULL;
