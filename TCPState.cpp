/*
 * TCPState.cpp
 *
 *  Created on: Dec 31, 2010
 *      Author: bogdan
 */

#include "TCPState.h"
#include <iostream>

void TCPState::ChangeState(TCPConnection *conn, TCPState *state)
{
	conn->ChangeState(state);
}
