#ifndef MCOP_LOOPBACKCONNECTION_H
#define MCOP_LOOPBACKCONNECTION_H

#include "connection.h"

/*
 * BC - Status (2000-09-30): LoopbackConnection
 *
 * Not part of the public API. Do NOT use it in your apps. No binary
 * compatibility guaranteed.
 */

namespace Arts {

class LoopbackConnection : public Connection
{
public:
	LoopbackConnection(const std::string& serverID);

	void qSendBuffer(Buffer *buffer);
	void drop();
	bool broken();
};

};

#endif /* MCOP_LOOPBACKCONNECTION_H */
