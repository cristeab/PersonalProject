/*
 * TCPConnection.h
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#ifndef TCPCONNECTION_H_
#define TCPCONNECTION_H_

/*
 *
 */
//context

class TCPOctetStream;
class TCPState;

class TCPConnection
{
public:
	TCPConnection();
	void ActiveOpen();
	void PassiveOpen();
	void Close();
	void Send();
	void Acknowledge();
	void Synchronize();
	void ProcessOctet(TCPOctetStream *stream)
	{
	}
	void Transmit(TCPOctetStream */*stream*/);
private:
	friend class TCPState;
	void ChangeState(TCPState *state);//only stated could change connection state
	TCPState *state_;
};


#endif /* TCPCONNECTION_H_ */
