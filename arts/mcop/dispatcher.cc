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

#include <config.h>
#include "dispatcher.h"
#include "startupmanager.h"
#include "unixconnection.h"
#include "tcpconnection.h"
#include "ifacerepo_impl.h"
#include "core.h"
#include <sys/time.h>
#include <stdio.h>
#include <sys/stat.h>
							  
Dispatcher *Dispatcher::_instance = 0;

Dispatcher::Dispatcher(IOManager *ioManager)
{
	assert(!_instance);
	_instance = this;

	generateServerID();

	if(ioManager)
	{
		_ioManager = ioManager;
		deleteIOManagerOnExit = false;
	}
	else
	{
		_ioManager = new StdIOManager;
		deleteIOManagerOnExit = true;
	}

	objectManager = new ObjectManager;

	unixServer = new UnixServer(this,serverID);
	if(!unixServer->running())
	{
		delete unixServer;
		fprintf(stderr,"MCOP Warning: couldn't start UnixServer\n");
		unixServer = 0;
	}

	tcpServer = new TCPServer(this);
	if(!tcpServer->running())
	{
		delete tcpServer;
		fprintf(stderr,"MCOP Warning: couldn't start TCPServer\n");
		tcpServer = 0;
	}

	_interfaceRepo = new InterfaceRepo_impl();
	_flowSystem = 0;

	StartupManager::startup();
}

Dispatcher::~Dispatcher()
{
	StartupManager::shutdown();

	if(_interfaceRepo)
	{
		delete _interfaceRepo;
		_interfaceRepo = 0;
	}
	if(unixServer)
	{
		delete unixServer;
		unixServer = 0;
	}

	if(tcpServer)
	{
		delete tcpServer;
		tcpServer = 0;
	}

	if(objectManager)
	{
		delete objectManager;
		objectManager = 0;
	}

	if(deleteIOManagerOnExit)
	{
		delete _ioManager;
		_ioManager = 0;
	}
	assert(_instance);
	_instance = 0;
}

InterfaceRepo *Dispatcher::interfaceRepo()
{
	return _interfaceRepo;
}

FlowSystem *Dispatcher::flowSystem()
{
	assert(_flowSystem);
	return _flowSystem;
}

void Dispatcher::setFlowSystem(FlowSystem *fs)
{
	assert(!_flowSystem);
	_flowSystem = fs;
}

Dispatcher *Dispatcher::the()
{
	assert(_instance);
	return _instance;
}

Buffer *Dispatcher::waitForResult(long requestID)
{
	Buffer *b;

	do {
		_ioManager->processOneEvent(true);
		b = requestResultPool[requestID];
	} while(!b);

	requestResultPool.releaseSlot(requestID);
	return b;
}

Buffer *Dispatcher::createRequest(long& requestID, long objectID, long methodID)
{
	Buffer *buffer = new Buffer;

	// write mcop header record
	buffer->writeLong(MCOP_MAGIC);
	buffer->writeLong(0);			// message length - to be patched later
	buffer->writeLong(mcopInvocation);

	// generate a request ID
	requestID = requestResultPool.allocSlot();

	// write invocation record
	buffer->writeLong(requestID);
	buffer->writeLong(objectID);
	buffer->writeLong(methodID);

	return buffer;
}

void Dispatcher::handle(Connection *conn, Buffer *buffer, long messageType)
{
	/**
	 * TODO: restrict incoming messages on unauthenticated connections
	 */
#ifdef DEBUG_IO
	printf("got a message %ld, %ld bytes in body\n",
			messageType,buffer->remaining());
#endif
	if(messageType == mcopInvocation) {
#ifdef DEBUG_MESSAGES
		printf("[got Invocation]\n");
#endif
		long requestID = buffer->readLong();
		long objectID = buffer->readLong();
		long methodID = buffer->readLong();

		Buffer *result = new Buffer;
		// write mcop header record
		result->writeLong(MCOP_MAGIC);
		result->writeLong(0);			// message length - to be patched later
		result->writeLong(mcopReturn);

		// write result record (returnCode is written by dispatch)
		result->writeLong(requestID);
	
		objectPool[objectID]->_dispatch(buffer,result,methodID);

		assert(!buffer->readError() && !buffer->remaining());
		delete buffer;

		result->patchLength();
		conn->qSendBuffer(result);
	}
	else if(messageType == mcopReturn)
	{
#ifdef DEBUG_MESSAGES
		printf("[got Return]\n");
#endif
		long requestID = buffer->readLong();
		requestResultPool[requestID] = buffer;
	}
	else if(messageType == mcopServerHello)
	{
#ifdef DEBUG_MESSAGES
		printf("[got ServerHello]\n");
#endif
		/*
		 * if we get a server hello, answer with a client hello
		 *
		 * currently, we always send noauth (because no authentication
		 * is implemented
		 */
		ServerHello h;
		h.readType(*buffer);
		assert(!buffer->readError() && !buffer->remaining());
		delete buffer;

		conn->setServerID(h.serverID);

		Buffer *helloBuffer = new Buffer;

		Header header(MCOP_MAGIC,0,mcopClientHello);
		header.writeType(*helloBuffer);
		ClientHello clientHello(serverID,"noauth","");
		clientHello.writeType(*helloBuffer);

		helloBuffer->patchLength();

		conn->qSendBuffer(helloBuffer);
	}
	else if(messageType == mcopClientHello)
	{
#ifdef DEBUG_MESSAGES
		printf("[got ClientHello]\n");
#endif
		/*
		 * currently, we accept each and every clienthello (no authentication
		 * supported
		 */
		ClientHello c;
		c.readType(*buffer);
		assert(!buffer->readError() && !buffer->remaining());
		delete buffer;

		conn->setServerID(c.serverID);

		Buffer *helloBuffer = new Buffer;

		Header header(MCOP_MAGIC,0,mcopAuthAccept);
		header.writeType(*helloBuffer);

		helloBuffer->patchLength();
		conn->qSendBuffer(helloBuffer);
		conn->setReady();
	}
	else if(messageType == mcopAuthAccept)
	{
#ifdef DEBUG_MESSAGES
		printf("[got AuthAccept]\n");
#endif
		conn->setReady();
	}
}

long Dispatcher::addObject(Object_skel *object)
{
	long objectID = objectPool.allocSlot();

	objectPool[objectID] = object;
	return objectID;
}

void Dispatcher::removeObject(long objectID)
{
	assert(objectPool[objectID]);
	objectPool.releaseSlot(objectID);
}

void Dispatcher::generateServerID()
{
	char buffer[1024];

	if(gethostname(buffer,1024) == 0)
	{
		serverID = buffer; 
	}
	else
	{
		serverID = "unknownhost";
	}
	serverID += ".";
	if(getdomainname(buffer,1024) == 0)
	{
		serverID += buffer; 
	}
	else
	{
		serverID += "nowhere.org";
	}
	sprintf(buffer,"-%04x-%08lx",getpid(),time(0));
	serverID += buffer;
}

string Dispatcher::objectToString(long objectID)
{
	Buffer b;
	ObjectReference or;

	or.serverID = serverID;
	or.objectID = objectID;

	// prefer a unix domainsocket connection over a plain tcp connection
	if(unixServer) or.urls.push_back(unixServer->url());
	if(tcpServer) or.urls.push_back(tcpServer->url());

	or.writeType(b);

	return b.toString("MCOP-Object");
}

bool Dispatcher::stringToObjectReference(ObjectReference& r, string s)
{
	if(strncmp(s.c_str(),"global:",7) == 0)
	{
		// if the object reference starts with "global:", it refers to
		// a global object which can be found with the objectManager

		string lookup = objectManager->getGlobalReference(&s.c_str()[7]);
		return stringToObjectReference(r,lookup);
	}


	Buffer b;
	if(!b.fromString(s,"MCOP-Object")) return false;

	r.readType(b);
	if(b.readError() || b.remaining()) return false;

	return true;
}

void *Dispatcher::connectObjectLocal(ObjectReference& reference,
													string interface)
{
	if(reference.serverID == serverID)
		return objectPool[reference.objectID]->_cast(interface);

	return 0;
}

Connection *Dispatcher::connectObjectRemote(ObjectReference& reference)
{
	list<Connection *>::iterator i;

	for(i=connections.begin(); i != connections.end();i++)
	{
		Connection *conn = *i;

		if(conn->isConnected(reference.serverID))
		{
			// fixme: we should check for the existence of the object
			// and increment a reference count or something like that
			return conn;
		}
	}

	/* try to connect the server */

	vector<string>::iterator ui;
	for(ui = reference.urls.begin(); ui != reference.urls.end(); ui++)
	{
		Connection *conn = 0;
		if(strncmp(ui->c_str(),"tcp:",4) == 0)
		{
			conn = new TCPConnection(*ui);
		}
		else if(strncmp(ui->c_str(),"unix:",5) == 0)
		{
			conn = new UnixConnection(*ui);
		}

		if(conn)
		{
			while(!conn->ready() && !conn->broken())
			{
				_ioManager->processOneEvent(true);
			}

			if(conn->ready())
			{
				connections.push_back(conn);

				assert(conn->isConnected(reference.serverID));
				return conn;
			}
			printf("bad luck: connecting server didn't work\n");
			
			// well - bad luck (connecting that server failed)
			delete conn;
		}
	}
	return 0;
}

void Dispatcher::run()
{
	_ioManager->run();
}

void Dispatcher::terminate()
{
	_ioManager->terminate();
}

void Dispatcher::initiateConnection(Connection *connection)
{
	vector<string> authProtocols;
	authProtocols.push_back("noauth");

	// to give authentication protocols a different seed each time
	// fixme: could be more bits

	char authSeed[1024];
	sprintf(authSeed,"%x",rand());

	Buffer *helloBuffer = new Buffer;

	Header header(MCOP_MAGIC,0,mcopServerHello);
	header.writeType(*helloBuffer);
	ServerHello serverHello(serverID,authProtocols,authSeed);
	serverHello.writeType(*helloBuffer);

	helloBuffer->patchLength();

	connection->qSendBuffer(helloBuffer);

	connections.push_back(connection);
}

void Dispatcher::handleConnectionClose(Connection *connection)
{
	/*
	 * FIXME:
	 *
	 * there may be error handling to do (e.g., check that the _stub's that
	 * still refer to that connection don't crash now).
	 */
	delete connection;
}
