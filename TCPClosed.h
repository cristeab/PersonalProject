/*
 * TCPClosed.h
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#ifndef TCPCLOSED_H_
#define TCPCLOSED_H_

#include "TCPState.h"
#include <cstddef>

class TCPConnection;


/*
 *
 */
class TCPClosed : public TCPState
{
public:
	TCPClosed();
	static TCPState* Instance();
	virtual void ActiveOpen(TCPConnection *conn);
	virtual void PassiveOpen(TCPConnection *conn);
private:
	static TCPState *instance_;
};

#endif /* TCPCLOSED_H_ */
