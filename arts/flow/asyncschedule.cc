#include "asyncschedule.h"

ASyncPort::ASyncPort(std::string name, void *ptr, long flags,
		StdScheduleNode* parent) : Port(name, ptr, flags, parent), pull(false)
{
	stream = (GenericAsyncStream *)ptr;
	stream->channel = this;
	stream->_notifyID = notifyID = parent->object()->_mkNotifyID();
}

//-------------------- GenericDataChannel interface -------------------------

void ASyncPort::setPull(int packets, int capacity)
{
	pullNotification.receiver = parent->object();
	pullNotification.ID = notifyID;
	pull = true;

	for(int i=0;i<packets;i++)
	{
		GenericDataPacket *packet = stream->createPacket(capacity);
		packet->useCount = 0;
		pullNotification.data = packet;
		NotificationManager::the()->send(pullNotification);
	}
}

void ASyncPort::endPull()
{
	pull = false;
	// TODO: maybe remove all pending pull packets here
}

void ASyncPort::processedPacket(GenericDataPacket *packet)
{
	cout << "port::processedPacket" << endl;
	assert(packet->useCount == 0);
	if(pull)
	{
		pullNotification.data = packet;
		NotificationManager::the()->send(pullNotification);
	}
	else
	{
		stream->freePacket(packet);
	}
}

void ASyncPort::sendPacket(GenericDataPacket *packet)
{
	cout << "port::sendPacket" << endl;

	if(packet->size > 0)
	{
		vector<Notification>::iterator i;
		for(i=subscribers.begin(); i != subscribers.end(); i++)
		{
			Notification n = *i;
			n.data = packet;
			packet->useCount++;
			cout << "sending notification " << n.ID << endl;
			NotificationManager::the()->send(n);
		}
	}
	else
	{
		stream->freePacket(packet);
	}
}

//----------------------- Port interface ------------------------------------

void ASyncPort::connect(Port *xsource)
{
	cout << "port(" << _name << ")::connect" << endl;

	ASyncPort *source = xsource->asyncPort();
	assert(source);
	addAutoDisconnect(xsource);

	Notification n;
	n.receiver = parent->object();
	n.ID = notifyID;
	source->subscribers.push_back(n);
}

void ASyncPort::disconnect(Port *xsource)
{
	cout << "port::disconnect" << endl;

	ASyncPort *source = xsource->asyncPort();
	assert(source);
	removeAutoDisconnect(xsource);

	// remove our subscription from the source object
	vector<Notification>::iterator si;
	for(si = source->subscribers.begin(); si != source->subscribers.end(); si++)
	{
		if(si->receiver == parent->object())
		{
			source->subscribers.erase(si);
			return;
		}
	}

	// there should have been exactly one, so this shouldn't be reached
	assert(false);
}

ASyncPort *ASyncPort::asyncPort()
{
	return this;
}
