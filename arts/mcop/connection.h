    /*

    Copyright (C) 1999 Stefan Westerfeld
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

    */

#ifndef CONNECTION_H
#define CONNECTION_H

#include "buffer.h"

class Connection {
protected:
	Buffer *rcbuf;
	bool receiveHeader;
	long remaining;
	long messageType;
	bool _ready;
	string serverID;

	/**
	 * If you don't want to handle message fragmentation yourself:
	 * 
	 * Call initReceive in your derived Connection constructor, ald
	 * receive as soon as you receive some data - the connection object
	 * will handle the rest (put the messages into buffers and send them
	 * to the dispatcher)
	 */
	void initReceive();
	void receive(unsigned char *data, long len);
public:
	Connection();
	virtual ~Connection();

	inline void setServerID(string serverID) { this->serverID = serverID; }
	inline bool isConnected(string s) { return (serverID == s); } 
	inline void setReady() { _ready = true; };
	inline bool ready() { return _ready; };
	virtual bool broken() = 0;
	virtual void qSendBuffer(Buffer *buffer) = 0;
};

#endif
