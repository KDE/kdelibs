#ifndef MCOP_LOOPBACKCONNECTION_H
#define MCOP_LOOPBACKCONNECTION_H

#include "connection.h"

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
