    /*

    Copyright (C) 2000,2001 Stefan Westerfeld
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

/* Since this file is a tad bit more complex, here is some basic documentation:

1) ASyncPort: There are asynchronous ports which are parts of the standard-
   flowsystem schedule nodes. Their lifetime starts whenever an asynchronous
   stream gets created by the flow system, and ends when the schedule node
   gets destroyed. Basically, an ASyncPort has two functions:

   * it is a "Port", which means that it gets connect(), disconnect() and
     other calls from the flowsystem

   * it is a "GenericDataChannel", which means that DataPackets can interact
     with it

   Although there will be ASyncPorts which only send data and ASyncPorts which
   only receive data (there are none that do both), there are no distinct
   classes for this.

2) Standard case: a DataPacket that gets transported over a datachannel locally:

   1. the user allocates himself a datapacket "packet"
   2. the user calls "packet->send()", which in turn calls
      ASyncPort::sendPacket(packet)
   3. the ASyncPort sends the DataPacket to every subscriber (incrementing the
      useCount) over the NotificationManager
   4. the NotificationManager delivers the DataPackets to the receiver
   5. eventually, the receiver confirms using "packet->processed()"
   6. the packet informs the ASyncPort::processedPacket()
   7. the packet is freed

variant (pulling):

   1. the user gets told by the ASyncPort: produce some data, here is a "packet"
   2. the user calls "packet->send()", which in turn calls
      ASyncPort::sendPacket(packet)
   3. the ASyncPort sends the DataPacket to every subscriber (incrementing the
      useCount) over the NotificationManager
   4. the NotificationManager delivers the DataPackets to the receiver
   5. eventually, the receiver confirms using "packet->processed()"
   6. the packet informs the ASyncPort::processedPacket()
   7. the ASyncPort restarts with 1.

3) Remote case: the remote case follows from the local case by adding two extra
things: one object that converts packets from their packet form to a message
(ASyncNetSend), and one object that converts packets from the message form
to a packet again. Effectively, the sending of a single packet looks like
this, then:

   1-S. the user allocates himself a datapacket "packet"
   2-S. the user calls "packet->send()", which in turn calls
        ASyncPort::sendPacket(packet)
   3-S. the ASyncPort sends the DataPacket to every subscriber (incrementing the
        useCount) over the NotificationManager
   4-S. the NotificationManager delivers the DataPackets to the ASyncNetSend
   5-S. the ASyncNetSend::notify method gets called, which in turn converts
        the packet to a network message
    
	... network transfer ...
   
   6-R. the ASyncNetReceive::receive method gets called - the method creates
        a new data packet, and sends it using the NotificationManager again
   7-R. the NotificationManager delivers the DataPacket to the receiver
   8-R. eventually, the receiver confirms using "packet->processed()"
   9-R. the packet informs the ASyncNetReceive::processedPacket() which
        frees the packet and tells the (remote) sender that it went all right

	... network transfer ...
 
   10-S. eventually, ASyncNetSend::processed() gets called, and confirms
         the packet using "packet->processed()"
   11-S. the packet informs the ASyncPort::processedPacket()
   12-S. the packet is freed

variant(pulling):

   works the same as in the local case by exchanging steps 1-S and 12-S

4) ownership:

   * ASyncPort: is owned by the Object which it is a part of, if the object
   dies, ASyncPort dies unconditionally

   * DataPacket: is owned by the GenericDataChannel they are propagated over,
   that is, the ASyncPort normally - however if the DataPacket is still in
   use (i.e. in state 5 of the local case), it will take responsibility to
   free itself once all processed() calls have been collected

   * ASyncNetSend, ASyncNetReceive: own each other, so that if the sender dies,
   the connection will die as well, and if the receiver dies, the same happens

*/

#undef DEBUG_ASYNC_TRANSFER

ASyncPort::ASyncPort(std::string name, void *ptr, long flags,
		StdScheduleNode* parent) : Port(name, ptr, flags, parent), pull(false)
{
	stream = (GenericAsyncStream *)ptr;
	stream->channel = this;
	stream->_notifyID = notifyID = parent->object()->_mkNotifyID();
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

	/* disconnect remote connections (if present): the following things will
	 * need to be ensured here, since we are being deleted:
     *
	 *  - the senders should not talk to us after our destructor
	 *  - all of our connections need to be disconnected
	 *  - every connection needs to be closed exactly once
     *
	 * (closing a connection can cause reentrancy due to mcop communication)
	 */
	while(!netSenders.empty())
		netSenders.front()->disconnect();

	FlowSystemReceiver receiver = netReceiver;
	if(!receiver.isNull())
		receiver.disconnect();
}

//-------------------- GenericDataChannel interface -------------------------

void ASyncPort::setPull(int packets, int capacity)
{
	pullNotification.receiver = parent->object();
	pullNotification.ID = notifyID;
	pullNotification.internal = 0;
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
	list<GenericDataPacket *>::iterator i = sent.begin();
	while(i != sent.end())
	{
		if(*i == packet)
		{
			count++;
			i = sent.erase(i);
		}
		else i++;
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
	bool sendOk = false;

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
			sendOk = true;
		}
	}

	if(sendOk)
		sent.push_back(packet);
	else
		stream->freePacket(packet);
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
	n.internal = 0;
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
void ASyncPort::addSendNet(ASyncNetSend *netsend)
{
	Notification n;
	n.receiver = netsend;
	n.ID = netsend->notifyID();
	n.internal = 0;
	subscribers.push_back(n);
	netSenders.push_back(netsend);
}

void ASyncPort::removeSendNet(ASyncNetSend *netsend)
{
	arts_return_if_fail(netsend != 0);
	netSenders.remove(netsend);

	vector<Notification>::iterator si;
	for(si = subscribers.begin(); si != subscribers.end(); si++)
	{
		if(si->receiver == netsend)
		{
			subscribers.erase(si);
			return;
		}
	}
	arts_warning("Failed to remove ASyncNetSend (%p) from ASyncPort", netsend);
}

void ASyncPort::setNetReceiver(ASyncNetReceive *receiver)
{
	arts_return_if_fail(receiver != 0);

	FlowSystemReceiver r = FlowSystemReceiver::_from_base(receiver->_copy());
	netReceiver = r;
}

void ASyncPort::disconnectRemote(const string& dest)
{
	list<ASyncNetSend *>::iterator i;

	for(i = netSenders.begin(); i != netSenders.end(); i++)
	{
		if((*i)->dest() == dest)
		{
			(*i)->disconnect();
			return;
		}
	}
	arts_warning("failed to disconnect %s in ASyncPort", dest.c_str());
}

ASyncNetSend::ASyncNetSend(ASyncPort *ap, const std::string& dest) : ap(ap)
{
	_dest = dest;
	ap->addSendNet(this);
}

ASyncNetSend::~ASyncNetSend()
{
	while(!pqueue.empty())
	{
		pqueue.front()->processed();
		pqueue.pop();
	}
	if(ap)
	{
		ap->removeSendNet(this);
		ap = 0;
	}
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

	/*
	 * since packets are delivered asynchronously, and since disconnection
	 * involves communication, it might happen that we get a packet without
	 * actually being connected any longer - in that case, silently forget it
	 */
	if(!receiver.isNull())
	{
		// put it into a custom data message and send it to the receiver
		Buffer *buffer = receiver._allocCustomMessage(receiveHandlerID);
		dp->write(*buffer);
		receiver._sendCustomMessage(buffer);
	}
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
	/* since disconnection will cause destruction (most likely immediate),
	 * we'll reference ourselves ...  */
	_copy();

	if(!receiver.isNull())
	{
		FlowSystemReceiver r = receiver;
		receiver = FlowSystemReceiver::null();
		r.disconnect();
	}
	if(ap)
	{
		ap->removeSendNet(this);
		ap = 0;
	}
	
	_release();
}

string ASyncNetSend::dest()
{
	return _dest;
}

/* dispatching function for custom message */

static void _dispatch_ASyncNetReceive_receive(void *object, Buffer *buffer)
{
	((ASyncNetReceive *)object)->receive(buffer);
}

ASyncNetReceive::ASyncNetReceive(ASyncPort *port, FlowSystemSender sender)
{
	port->setNetReceiver(this);
	stream = port->receiveNetCreateStream();
	stream->channel = this;
	this->sender = sender;
	/* stream->_notifyID = _mkNotifyID(); */

	gotPacketNotification.ID = port->receiveNetNotifyID();
	gotPacketNotification.receiver = port->receiveNetObject();
	gotPacketNotification.internal = 0;
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

void ASyncNetReceive::sendPacket(GenericDataPacket *)
{
	assert(false);
}

void ASyncNetReceive::setPull(int, int)
{
	assert(false);
}

void ASyncNetReceive::endPull()
{
	assert(false);
}
