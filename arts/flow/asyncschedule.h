    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#ifndef ASYNCSCHEDULE_H
#define ASYNCSCHEDULE_H

#include "synthschedule.h"
#include "datapacket.h"

class ASyncNetSend : public FlowSystemSender_skel
{
protected:
	queue<GenericDataPacket *> pqueue;
	FlowSystemReceiver_var receiver;
	long receiveHandlerID;

public:
	long notifyID();

	/* this overwrites the Object::notify function */
	void notify(const Notification& notification);
	void processed();
	void setReceiver(FlowSystemReceiver *receiver);
};

class ASyncNetReceive : public FlowSystemReceiver_skel,
						public GenericDataChannel
{
protected:
	GenericAsyncStream *stream;
	FlowSystemSender_var sender;
	Notification gotPacketNotification;
	long _receiveHandlerID;

public:
	ASyncNetReceive(ASyncPort *port, FlowSystemSender *sender);

	// GenericDataChannel interface
	void processedPacket(GenericDataPacket *packet);
	void sendPacket(GenericDataPacket *packet);
	void setPull(int packets, int capacity);
	void endPull();

	// FlowSystemReceiver interface
	
	long receiveHandlerID();
	void receive(Buffer *buffer);	// custom data receiver
};

class ASyncPort :public Port, public GenericDataChannel {
protected:
	long notifyID;
	std::vector<Notification> subscribers;

	GenericAsyncStream *stream;

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

	void connect(Port *port);
	void disconnect(Port *port);
	ASyncPort *asyncPort();

	// Network transparency
	
	void sendNet(ASyncNetSend *netsend);
	long receiveNetNotifyID();
	GenericAsyncStream *receiveNetCreateStream();
	NotificationClient *receiveNetObject();
};
#endif /* ASYNCSCHEDULE_H */
