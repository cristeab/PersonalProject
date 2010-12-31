/*
 * TCPEstablished.cpp
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#include "TCPEstablished.h"
#include "TCPListen.h"
#include "TCPClosed.h"
#include <iostream>

TCPState* TCPEstablished::Instance()
{
	if (NULL == instance_)
	{
		instance_ = new TCPEstablished();
	}
	return instance_;
}

void TCPEstablished::Transmit(TCPConnection *conn, TCPOctetStream */*stream*/)
{
	std::cout << "transmitting from TCPEstablished" << std::endl;
	ChangeState(conn, TCPListen::Instance());
}

void TCPEstablished::Close(TCPConnection *conn)
{
	std::cout << "closing TCPEstablished" << std::endl;
	ChangeState(conn, TCPClosed::Instance());
}

TCPState* TCPEstablished::instance_ = NULL;
