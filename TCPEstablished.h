/*
 * TCPEstablished.h
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#ifndef TCPESTABLISHED_H_
#define TCPESTABLISHED_H_

#include "TCPState.h"
#include <cstddef>

/*
 *
 */
//concrete state (singleton)
class TCPEstablished : public TCPState
{
public:
	static TCPState *Instance();
	virtual void Transmit(TCPConnection *conn, TCPOctetStream */*stream*/);
	virtual void Close(TCPConnection *conn);
private:
	static TCPState *instance_;
};

#endif /* TCPESTABLISHED_H_ */
