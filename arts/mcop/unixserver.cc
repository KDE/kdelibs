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

#include <config.h>
#include "mcoputils.h"
#include "unixserver.h"
#include "socketconnection.h"
#include "dispatcher.h"
#define queue cqueue
#include <arpa/inet.h>
#undef queue
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>                                                      
#include <stdio.h>
#include <errno.h>

using namespace std;

bool UnixServer::running()
{
	return socketOk;
}

UnixServer::UnixServer(Dispatcher *dispatcher, string serverID)
{
	this->dispatcher = dispatcher;

	socketOk = initSocket(serverID);
	if(socketOk) {
		IOManager *iom = dispatcher->ioManager();
		iom->watchFD(theSocket,IOType::read|IOType::except,this);
	}
}

UnixServer::~UnixServer()
{
	if(socketOk) {
		IOManager *iom = dispatcher->ioManager();
		iom->remove(this,IOType::read|IOType::except);
		close(theSocket);
		unlink(xserverpath.c_str());
	}
}

bool UnixServer::initSocket(string serverID)
{
    struct sockaddr_un socket_addr;

	theSocket = socket(AF_UNIX,SOCK_STREAM,0);
	if(theSocket < 0)
	{
		fprintf(stderr,"MCOP UnixServer: can't create a socket\n");
		return false;
	}

	if(fcntl(theSocket,F_SETFL,O_NONBLOCK)<0)
	{
		fprintf(stderr,"MCOP UnixServer: can't initialize non blocking I/O\n");
		close(theSocket);
		return false;
	}

	string pathname = MCOPUtils::createFilePath(serverID);
    socket_addr.sun_family = AF_UNIX;
	strncpy(socket_addr.sun_path,pathname.c_str(),108);
	socket_addr.sun_path[107] = 0;

	xserverpath = socket_addr.sun_path;

    if ( bind( theSocket, (struct sockaddr *) &socket_addr,
               sizeof(struct sockaddr_un) ) < 0 )
    {
		fprintf(stderr,"MCOP UnixServer: can't bind to file \"%s\"\n",
			pathname.c_str());
        close(theSocket);
		return false;
    }

	if(listen(theSocket,16) < 0)
	{
		fprintf(stderr,"MCOP UnixServer: can't listen on the socket\n");
        close(theSocket);
		return false;
	}
	return true;
}

string UnixServer::url()
{
	return string("unix:") + xserverpath;
}

void UnixServer::notifyIO(int fd, int types)
{
	assert(fd == theSocket);
	assert(socketOk);

	printf("UnixManager: got notifyIO\n");
	if(types & IOType::read)
	{
		int clientfd;
		struct sockaddr_un incoming;
		ksize_t size_in = sizeof(struct sockaddr_un);

		clientfd = accept(theSocket, (struct sockaddr*) &incoming, &size_in );
		if(clientfd > 0)
		{
			// non blocking I/O
			int flags = fcntl (fd, F_GETFL, 0);
			assert (flags != -1);

			int nbl_result = fcntl (fd, F_SETFL, flags | O_NONBLOCK);
			assert(nbl_result >= 0);                   
	
			Dispatcher::the()->initiateConnection(
				new SocketConnection(clientfd));
		}
	}
	if(types & IOType::write) printf("- write\n");
	if(types & IOType::except) printf("- except\n");
}
