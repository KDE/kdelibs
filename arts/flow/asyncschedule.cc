    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
   
    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    */

#include "asyncschedule.h"
#include "debug.h"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace Arts;

#undef DEBUG_ASYNC_TRANSFER

ASyncPort::ASyncPort(std::string name, void *ptr, long flags,
		StdScheduleNode* parent) : Port(name, ptr, flags, parent), pull(false)
{
	stream = (GenericAsyncStream *)ptr;
	stream->channel = this;
	stream->_notifyID = notifyID = parent->object()->_mkNotifyID();

	sender = FlowSystemSender::null();
}

ASyncPort::~ASyncPort()
{
	/* 
	 * tell all outstanding packets that we don't exist any longer, so that
	 * if they feel like they need to confirm they have been processed now,
	 * they don't talk to an no longer existing object about it
	 */
	while(!sent.empty())
	{
		sent.front()->channel = 0;
		sent.pop_front();
	}

	/*
	 * disconnect remote connection (if there was one)
	 * TODO: remote multicasting doesn't work (probably also not too
	 * important)
	 */
	if(!sender.isNull())
		sender.disconnect();
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
	int count = 0;
	list<GenericDataPacket *>::iterator i, nexti;
	for(i=sent.begin(); i != sent.end(); i = nexti)
	{
		nexti = i; nexti++;

		if(*i == packet)
		{
			count++;
			sent.erase(i);
		}
	}
	assert(count == 1);

#ifdef DEBUG_ASYNC_TRANSFER
	cout << "port::processedPacket" << endl;
#endif
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
#ifdef DEBUG_ASYNC_TRANSFER
	cout << "port::sendPacket" << endl;
#endif

	if(packet->size > 0)
	{
		vector<Notification>::iterator i;
		for(i=subscribers.begin(); i != subscribers.end(); i++)
		{
			Notification n = *i;
			n.data = packet;
			packet->useCount++;
#ifdef DEBUG_ASYNC_TRANSFER
			cout << "sending notification " << n.ID << endl;
#endif
			NotificationManager::the()->send(n);
			sent.push_back(packet);
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
	arts_debug("port(%s)::connect",_name.c_str());

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
	arts_debug("port::disconnect");

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

GenericAsyncStream *ASyncPort::receiveNetCreateStream()
{
	return stream->createNewStream();
}

NotificationClient *ASyncPort::receiveNetObject()
{
	return parent->object();
}

long ASyncPort::receiveNetNotifyID()
{
	return notifyID;
}

// Network transparency
void ASyncPort::sendNet(ASyncNetSend *netsend)
{
	Notification n;
	n.receiver = netsend;
	n.ID = netsend->notifyID();
	subscribers.push_back(n);
	sender = FlowSystemSender::_from_base(netsend->_copy());
}

long ASyncNetSend::notifyID()
{
	return 1;
}

void ASyncNetSend::notify(const Notification& notification)
{
	// got a packet?
	assert(notification.ID == notifyID());
	GenericDataPacket *dp = (GenericDataPacket *)notification.data;
	pqueue.push(dp);

	// put it into a custom data message and send it to the receiver
	Buffer *buffer = receiver._allocCustomMessage(receiveHandlerID);
	dp->write(*buffer);
	receiver._sendCustomMessage(buffer);
}

void ASyncNetSend::processed()
{
	assert(!pqueue.empty());
	pqueue.front()->processed();
	pqueue.pop();
}

void ASyncNetSend::setReceiver(FlowSystemReceiver newReceiver)
{
	receiver = newReceiver;
	receiveHandlerID = newReceiver.receiveHandlerID();
}

void ASyncNetSend::disconnect()
{
	if(!receiver.isNull())
	{
		FlowSystemReceiver r = receiver;
		receiver = FlowSystemReceiver::null();
		r.disconnect();
	}
}

/* dispatching function for custom message */

static void _dispatch_ASyncNetReceive_receive(void *object, Buffer *buffer)
{
	((ASyncNetReceive *)object)->receive(buffer);
}

ASyncNetReceive::ASyncNetReceive(ASyncPort *port, FlowSystemSender sender)
{
	stream = port->receiveNetCreateStream();
	stream->channel = this;
	this->sender = sender;
	/* stream->_notifyID = _mkNotifyID(); */

	gotPacketNotification.ID = port->receiveNetNotifyID();
	gotPacketNotification.receiver = port->receiveNetObject();
	_receiveHandlerID =
		_addCustomMessageHandler(_dispatch_ASyncNetReceive_receive,this);
}

ASyncNetReceive::~ASyncNetReceive()
{
	/* tell outstanding packets that we don't exist any longer */
	while(!sent.empty())
	{
		sent.front()->channel = 0;
		sent.pop_front();
	}
	delete stream;
}

long ASyncNetReceive::receiveHandlerID()
{
	return _receiveHandlerID;
}

void ASyncNetReceive::receive(Buffer *buffer)
{
	GenericDataPacket *dp = stream->createPacket(512);
	dp->read(*buffer);
	dp->useCount = 1;
	gotPacketNotification.data = dp;
	NotificationManager::the()->send(gotPacketNotification);
	sent.push_back(dp);
}

/*
 * It will happen that this routine is called in time critical situations,
 * such as: while audio calculation is running, and must be finished in
 * time. The routine is mostly harmless, because sender->processed() is
 * a oneway function, which just queues the buffer for sending and returns
 * back, so it should return at once.
 *
 * However there is an exception upon first call: when sender->processed()
 * is called for the first time, the method processed has still to be looked
 * up. Thus, a synchronous call to _lookupMethod is made. That means, upon
 * first call, the method will send out an MCOP request and block until the
 * remote process tells that id.
 */
void ASyncNetReceive::processedPacket(GenericDataPacket *packet)
{
	/*
	 * HACK! Upon disconnect, strange things will happen. One of them is
	 * that we might, for the reason of not being referenced any longer,
	 * cease to exist without warning. Another is that our nice "sender"
	 * reference will get a null reference without warning, see disconnect
	 * code (which will cause the attached stub to also disappear). As
	 * those objects (especially the stub) are not prepared for not
	 * being there any more in the middle of whatever they do, we here
	 * explicitely reference us, and them, *again*, so that no evil things
	 * will happen. A general solution for this would be garbage collection
	 * in a timer, but until this is implemented (if it ever will become
	 * implemented), we'll live with this hack.
	 */
	_copy();
	sent.remove(packet);
	stream->freePacket(packet);
	if(!sender.isNull())
	{
		FlowSystemSender xsender = sender;
		xsender.processed();
	}
	_release();
}

void ASyncNetReceive::disconnect()
{
	if(!sender.isNull())
	{
		FlowSystemSender s = sender;
		sender = FlowSystemSender::null();
		s.disconnect();
	}
}

void ASyncNetReceive::sendPacket(GenericDataPacket *packet)
{
	assert(false);
}

void ASyncNetReceive::setPull(int packets, int capacity)
{
	assert(false);
}

void ASyncNetReceive::endPull()
{
	assert(false);
}
