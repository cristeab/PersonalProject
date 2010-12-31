/*
 * TCPState.h
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#ifndef TCPSTATE_H_
#define TCPSTATE_H_

#include "TCPConnection.h"

/*
 *
 */
//state interface
class TCPState
{
public:
	virtual void Transmit(TCPConnection *conn, TCPOctetStream *str)
	{}
	virtual void ActiveOpen(TCPConnection *conn)
	{}
	virtual void PassiveOpen(TCPConnection *conn)
	{}
	virtual void Close(TCPConnection *conn)
	{}
	virtual void Synchronize(TCPConnection *conn)
	{}
	virtual void Acknowledge(TCPConnection *conn)
	{}
	virtual void Send(TCPConnection *conn)
	{}
	virtual ~TCPState()
	{}
protected:
	void ChangeState(TCPConnection *conn, TCPState *state);
};

#endif /* TCPSTATE_H_ */
