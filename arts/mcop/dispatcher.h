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

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "common.h"
#include "pool.h"
#include "iomanager.h"
#include "tcpserver.h"
#include "unixserver.h"
#include "connection.h"
#include "flowsystem.h"

#include <deque>
#include <stack>
#include <vector>
#include <list>

class ObjectReference;
class InterfaceRepo;
class FlowSystem;
class ObjectManager;

class Dispatcher {
protected:
	static Dispatcher *_instance;

	Pool<Buffer> requestResultPool;
	Pool<class Object_skel> objectPool;
	list<Connection *> connections;

	string serverID;
	void generateServerID();	// helper function

	bool deleteIOManagerOnExit;

	TCPServer *tcpServer;
	UnixServer *unixServer;
	IOManager *_ioManager;
	InterfaceRepo *_interfaceRepo;
	FlowSystem *_flowSystem;
	ObjectManager *objectManager;

public:
	Dispatcher(IOManager *ioManager = 0);
	~Dispatcher();

	static Dispatcher *the();
	inline IOManager *ioManager() { return _ioManager; };
	InterfaceRepo *interfaceRepo();
	FlowSystem *flowSystem();
	void setFlowSystem(FlowSystem *fs);

	void refillRequestIDs();

	Buffer *waitForResult(long requestID);		// blocking wait for result
	Buffer *createRequest(long& requestID, long objectID, long methodID);

	// processes messages
	void handle(Connection *conn, Buffer *buffer, long messageType);

	/**
	 * object registration
	 * 
	 * do not call manually, this is used by the Object_skel constructor
	 * to register itself
	 */
	long addObject(Object_skel *object);

	/**
	 * object deregistration
	 *
	 * do not call manually, this is called by the Object_skel destructor
	 * as soon as the object is deleted
	 */
	void removeObject(long objectID);

	/**
	 * connects to a local object, returning a readily casted "interface" *
	 * if success, or NULL if this object is not local
	 */
	void *connectObjectLocal(ObjectReference& reference, string interface);

	/**
	 * connects to a remote object, establishing a connection if necessary
	 *
	 * returns NULL if connecting fails or object isn't present or whatever
	 * else could go wrong
	 */
	Connection *connectObjectRemote(ObjectReference& reference);

	/**
	 * stringification of objects (only used by the object implementations,
	 * which in turn provide a _toString method)
	 */
	string objectToString(long objectID);

	/**
	 * destringification of objects (mainly used by the object implementations,
	 * which in turn provide a _fromString method)
	 */
	bool stringToObjectReference(ObjectReference& r, string s);

	/**
	 * main loop
	 */
	void run();

	/**
	 * this function quits the main loop (it must be running, of course)
	 * and causes the run() function to return.
	 */
	void terminate();

	/**
	 * Is called by the transport services as soon as a new connection is
	 * created - this will send authentication request, etc.
	 */
	void initiateConnection(Connection *connection);

	/**
	 * Is called by the Connection as soon as the connection goes down,
	 * which may happen due to a normal cause (client exits), and may
	 * happen if an error occurs as well (network down)
	 */
	void handleConnectionClose(Connection *connection);
};

#endif
