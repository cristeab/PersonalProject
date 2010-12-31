/*
 * TCPListen.h
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#ifndef TCPLISTEN_H_
#define TCPLISTEN_H_

#include "TCPState.h"
#include <cstddef>

/*
 *
 */
class TCPListen : public TCPState
{
public:
	TCPListen();
	static TCPState* Instance();
	virtual void Send(TCPConnection */*conn*/);
	void Close(TCPConnection */*conn*/);
private:
	static TCPState *instance_;
};

#endif /* TCPLISTEN_H_ */
