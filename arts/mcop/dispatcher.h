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

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "common.h"
#include "pool.h"
#include "iomanager.h"
#include "tcpserver.h"
#include "unixserver.h"
#include "connection.h"
#include "notification.h"

#include <deque>
#include <stack>
#include <vector>
#include <list>

/*
 * BC - Status (2000-09-30): Dispatcher
 *
 * This is part of the public API (and interacts with generated code) and
 * has to be kept binary compatible.
 */

namespace Arts {

class ObjectReference;
class FlowSystem_impl;
class GlobalComm;
class InterfaceRepo;
class ObjectManager;
class Object_skel;
class ReferenceClean;
class DelayedReturn;

extern "C" {
  typedef void (*mcop_sighandler)(int); 
}

class Dispatcher {
private:
	class DispatcherPrivate *d;

protected:
	static Dispatcher *_instance;

	Pool<Buffer> requestResultPool;
	Pool<Object_skel> objectPool;
	std::list<Connection *> connections;

	std::string serverID;
	void generateServerID();	// helper function

	bool deleteIOManagerOnExit;

	TCPServer *tcpServer;
	UnixServer *unixServer;
	IOManager *_ioManager;
	FlowSystem_impl *_flowSystem;
	ObjectManager *objectManager;
	ReferenceClean *referenceClean;
	NotificationManager *notificationManager;

	mcop_sighandler orig_sigpipe;		// original signal handler for SIG_PIPE
	Connection *_activeConnection;		// internal use only (for refcounting)

	/**
	 * connects to a given URL
	 *
	 * @returns a valid connection, or a 0 pointer on error
	 */
	Connection *connectUrl(const std::string& url);

public:
	enum StartServer {
		noServer = 0,					// don't be a server
		startUnixServer = 1,			// listen on a unix domain socket
		startTCPServer = 2,				// listen on a tcp port
		noAuthentication = 4			// don't require authentication
	};

	Dispatcher(IOManager *ioManager = 0, StartServer startServer = noServer);
	~Dispatcher();

	static Dispatcher *the();
	inline IOManager *ioManager() { return _ioManager; };
	InterfaceRepo interfaceRepo();
	FlowSystem_impl *flowSystem();
	Pool<Object_skel> activeObjectPool() { return objectPool; }
	GlobalComm globalComm();
	void setFlowSystem(FlowSystem_impl *fs);

	void refillRequestIDs();

	// blocking wait for result
	Buffer *waitForResult(long requestID,Connection *connection);

	// request creation for oneway and twoway requests
	Buffer *createRequest(long& requestID, long objectID, long methodID);
	Buffer *createOnewayRequest(long objectID, long methodID);

	// processes messages
	void handle(Connection *conn, Buffer *buffer, long messageType);

	/*
	 * special hook to handle corrupt messages
	 */
	void handleCorrupt(Connection *conn);

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
	void *connectObjectLocal(ObjectReference& reference, std::string interface);

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
	std::string objectToString(long objectID);

	/**
	 * destringification of objects (mainly used by the object implementations,
	 * which in turn provide a _fromString method)
	 */
	bool stringToObjectReference(ObjectReference& r, std::string s);

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

	/**
	 * Locks the dispatcher. Whenever you want to do anything with any kind
	 * of aRts object, you will hold a lock on Arts::Dispatcher. There is
	 * only one exception to the rule, and that is: you don't have to lock
	 * the dispatcher when the lock is already held.
	 *
	 * Generally, that is very often the case. Typical situations where you
	 * don't need to lock() the Dispatcher are:
	 *
	 *  @li you receive a callback from the IOManager (timer or fd)
	 *  @li you get call due to some MCOP request
	 *  @li you are called from the NotificationManager
	 *  @li you are called from the FlowSystem (calculateBlock)
	 */
	static void lock();

	/**
	 * Unlocks the dispatcher. Do this to release a lock you hold on
	 * the Arts::Dispatcher.
	 */
	static void unlock();

	/**
	 * Wakes the dispatcher - normally, the dispatcher will go to sleep in
	 * it's event loop, when nothing is to be done. If you change things from
	 * another thread, like: add a new Timer, write something to a connection,
	 * and so on, the dispatcher will not notice it. To wake the dispatcher
	 * up, this call is provided.
	 */
	static void wakeUp();

	/**
	 * - internal usage only -
	 *
	 * this will return the Connection the last request came from
	 */
	Connection *activeConnection();

	/**
	 * - internal usage only -
	 *
	 * this will return a loopback Connection for sending requests to myself
	 */
	Connection *loopbackConnection();

	/**
	 * - internal usage only -
	 *
	 * this will cause a function to return from a request later
	 * @see DelayedReturn
	 */
	DelayedReturn *delayReturn();	

	/**
	 * - internal usage only -
	 *
	 * gets object with a given ID
	 * @returns the object if successful, 0 otherwise
	 */
	Object_skel *getLocalObject(long ID);

	/**
	 * reloads the trader data (do this if things have been added/changed
	 * there)
	 */
	void reloadTraderData();
};
};
#endif
