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

#include <config.h>
#include "mcoputils.h"
#include "unixserver.h"
#include "socketconnection.h"
#include "dispatcher.h"
#include "debug.h"
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
using namespace Arts;

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

	theSocket = socket(PF_UNIX,SOCK_STREAM,0);
	if(theSocket < 0)
	{
		arts_warning("MCOP UnixServer: can't create a socket");
		return false;
	}

	if(fcntl(theSocket,F_SETFL,O_NONBLOCK)<0)
	{
		arts_warning("MCOP UnixServer: can't initialize non blocking I/O");
		close(theSocket);
		return false;
	}

	int maxlen = sizeof(socket_addr.sun_path);
	string pathname = MCOPUtils::createFilePath(serverID);

    socket_addr.sun_family = AF_UNIX;
	strncpy(socket_addr.sun_path,pathname.c_str(),maxlen);
	socket_addr.sun_path[maxlen-1] = 0;

	xserverpath = socket_addr.sun_path;

    if ( bind( theSocket, (struct sockaddr *) &socket_addr,
               sizeof(struct sockaddr_un) ) < 0 )
    {
		arts_warning("MCOP UnixServer: can't bind to file \"%s\"",
			pathname.c_str());
        close(theSocket);
		return false;
    }

	if(listen(theSocket,16) < 0)
	{
		arts_warning("MCOP UnixServer: can't listen on the socket");
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

	arts_debug("UnixManager: got notifyIO");
	if(types & IOType::read)
	{
		int clientfd;
		struct sockaddr_un incoming;
		socklen_t size_in = sizeof(struct sockaddr_un);

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
	arts_assert((types & IOType::write) == 0);
	arts_assert((types & IOType::except) == 0);
}
