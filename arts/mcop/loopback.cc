#include "loopback.h"
#include "core.h"

using namespace Arts;
using namespace std;

LoopbackConnection::LoopbackConnection(const std::string& serverID)
{
	setServerID(serverID);
	_connState = established;
}

void LoopbackConnection::qSendBuffer(Buffer *buffer)
{
	assert(buffer->size() >= 12);

	long magic = buffer->readLong();
	assert(magic == MCOP_MAGIC);

	/* messageSize, irrelevant here, since we've got the buffer already */
	buffer->readLong();	
	long messageType = buffer->readLong();

	Dispatcher::the()->handle(this,buffer,messageType);
}

void LoopbackConnection::drop()
{
	assert(false);
}

bool LoopbackConnection::broken()
{
	return false;
}
