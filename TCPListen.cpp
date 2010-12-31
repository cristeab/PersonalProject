/*
 * TCPListen.cpp
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#include "TCPListen.h"
#include "TCPEstablished.h"
#include "TCPClosed.h"
#include <iostream>

class TCPConnection;

TCPListen::TCPListen()
{
	std::cout << "TCPListen" << std::endl;
}

TCPState* TCPListen::Instance()
{
	if (NULL == instance_)
	{
		instance_ = new TCPListen();
	}
	return instance_;
}

void TCPListen::Send(TCPConnection *conn)
{
	std::cout << "sending from TCPListen" << std::endl;
	ChangeState(conn, TCPEstablished::Instance());
}

void TCPListen::Close(TCPConnection *conn)
{
	std::cout << "closing from TCPListen" << std::endl;
	ChangeState(conn, TCPClosed::Instance());
}

TCPState* TCPListen::instance_ = NULL;
