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

#ifndef ASYNCSCHEDULE_H
#define ASYNCSCHEDULE_H

#include "synthschedule.h"
#include "datapacket.h"

#include <queue>

namespace Arts {

class ASyncNetSend : public FlowSystemSender_skel
{
protected:
	std::queue<GenericDataPacket *> pqueue;
	FlowSystemReceiver receiver;
	long receiveHandlerID;

public:
	long notifyID();

	/* this overwrites the Object::notify function */
	void notify(const Notification& notification);
	void processed();
	void setReceiver(FlowSystemReceiver receiver);
	void disconnect();
};

class ASyncNetReceive : public FlowSystemReceiver_skel,
						public GenericDataChannel
{
protected:
	GenericAsyncStream *stream;
	FlowSystemSender sender;
	Notification gotPacketNotification;
	list<GenericDataPacket *> sent;
	long _receiveHandlerID;

public:
	ASyncNetReceive(ASyncPort *port, FlowSystemSender sender);
	~ASyncNetReceive();

	// GenericDataChannel interface
	void processedPacket(GenericDataPacket *packet);
	void sendPacket(GenericDataPacket *packet);
	void setPull(int packets, int capacity);
	void endPull();

	// FlowSystemReceiver interface
	
	long receiveHandlerID();
	void disconnect();
	void receive(Buffer *buffer);	// custom data receiver
};

class ASyncPort :public Port, public GenericDataChannel {
protected:
	long notifyID;
	std::vector<Notification> subscribers;
	list<GenericDataPacket *> sent;

	GenericAsyncStream *stream;
	FlowSystemSender sender;

	bool pull;
	Notification pullNotification;

public:
	// GenericDataChannel interface
	void processedPacket(GenericDataPacket *packet);
	void sendPacket(GenericDataPacket *packet);
	void setPull(int packets, int capacity);
	void endPull();

	// Port interface
	ASyncPort(std::string name, void *ptr, long flags, StdScheduleNode* parent);
	~ASyncPort();

	void connect(Port *port);
	void disconnect(Port *port);
	ASyncPort *asyncPort();

	// Network transparency
	
	void sendNet(ASyncNetSend *netsend);
	long receiveNetNotifyID();
	GenericAsyncStream *receiveNetCreateStream();
	NotificationClient *receiveNetObject();
};

};

#endif /* ASYNCSCHEDULE_H */
