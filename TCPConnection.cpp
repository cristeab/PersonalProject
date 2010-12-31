/*
 * TCPConnection.cpp
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#include <iostream>
#include "TCPConnection.h"
#include "TCPState.h"
#include "TCPClosed.h"

TCPConnection::TCPConnection()
{
	state_ = TCPClosed::Instance();
	std::cout << "TCPConnection" << std::endl;
}

void TCPConnection::ActiveOpen()
{
	state_->ActiveOpen(this);//allows to change the state if needed
}

void TCPConnection::PassiveOpen()
{
	state_->PassiveOpen(this);
}

void TCPConnection::Close()
{
	state_->Close(this);
}

void TCPConnection::Send()
{
	state_->Send(this);
}

void TCPConnection::Acknowledge()
{
	state_->Acknowledge(this);
}

void TCPConnection::Synchronize()
{
	state_->Synchronize(this);
}

void TCPConnection::ChangeState(TCPState *state)
{
	state_ = state;
}

void TCPConnection::Transmit(TCPOctetStream *stream)
{
	state_->Transmit(this, stream);
}
