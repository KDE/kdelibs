#include "asyncschedule.h"

ASyncPort::ASyncPort(std::string name, void *ptr, long flags,
			StdScheduleNode* parent) : Port(name, ptr, flags, parent)
{
	GenericAsyncStream *as = (GenericAsyncStream *)ptr;
	as->channel = this;
	as->_notifyID = notifyID = parent->object()->_mkNotifyID();
}

void ASyncPort::processedPacket(GenericDataPacket *)
{
	cout << "port::processedPacket" << endl;
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
		// should probably not use delete in here
		delete packet;
	}
}

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
	removeAutoDisconnect(xsource);
}

ASyncPort *ASyncPort::asyncPort()
{
	return this;
}
