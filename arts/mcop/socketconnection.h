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

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#ifndef SOCKETCONNECTION_H
#define SOCKETCONNECTION_H

#include "buffer.h"
#include "iomanager.h"
#include "connection.h"
#include <list>
#include <string>

class SocketConnection :public Connection, public IONotify {
protected:
	string serverID;
	int fd;
	bool _broken;

	list<Buffer *> pending;
	void writeBuffer(Buffer *buffer);

	SocketConnection();

public:
	SocketConnection(int fd);

	void qSendBuffer(Buffer *buffer);
	void notifyIO(int fd, int types);

	bool broken();
};

#endif /* SOCKETCONNECTION_H */
