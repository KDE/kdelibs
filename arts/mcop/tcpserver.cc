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
#include "tcpserver.h"
#include "socketconnection.h"
#include "dispatcher.h"
#define queue cqueue
#include <arpa/inet.h>
#undef queue
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "mcoputils.h"
#include "debug.h"

using namespace std;
using namespace Arts;

bool TCPServer::running()
{
	return socketOk;
}

TCPServer::TCPServer(Dispatcher *dispatcher)
{
	this->dispatcher = dispatcher;

	socketOk = initSocket();
	if(socketOk) {
		IOManager *iom = dispatcher->ioManager();
		iom->watchFD(theSocket,IOType::read|IOType::except,this);
	}
}

TCPServer::~TCPServer()
{
	if(socketOk)
	{
		IOManager *iom = dispatcher->ioManager();
		iom->remove(this,IOType::read|IOType::except);
		close(theSocket);
	}
}

static int TCPServerPort = 0;

void TCPServer::setPort(int port)
{
	TCPServerPort = port;
}

bool TCPServer::initSocket()
{
    struct sockaddr_in socket_addr;

	theSocket = socket(PF_INET,SOCK_STREAM,0);
	if(theSocket < 0)
	{
		arts_warning("MCOP TCPServer: can't create a socket");
		return false;
	}

	if(fcntl(theSocket,F_SETFL,O_NONBLOCK)<0)
	{
		arts_warning("MCOP TCPServer: can't initialize non blocking I/O");
		close(theSocket);
		return false;
	}

    if (TCPServerPort)
	{
		int optval = 1;
		if(setsockopt (theSocket, SOL_SOCKET, SO_REUSEADDR,
		               (const char *)&optval, sizeof (optval)) < 0)
		{
			arts_warning("MCOP TCPServer: can't set adress reuse");
			close(theSocket);
			return false;
		}
	}

#ifdef TCP_NODELAY
	// enable TCP sending without nagle algorithm - this sends out requests
	// faster, because  they are not queued in the hope that more data will
	// need to be sent soon
	int on = 1;
	if(setsockopt(theSocket, IPPROTO_TCP, TCP_NODELAY,
				  (char *)&on, sizeof(on)) < 0)
	{
		arts_debug("couldn't set TCP_NODELAY on socket %d\n", theSocket);
	}
#endif

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons( TCPServerPort );	// 0 = choose port freely
    socket_addr.sin_addr.s_addr = htonl( inet_addr("0.0.0.0") );
    if ( bind( theSocket, (struct sockaddr *) &socket_addr,
               sizeof(struct sockaddr_in) ) < 0 )
    {
		arts_warning("MCOP TCPServer: can't bind to port/address");
        close(theSocket);
		return false;
    }

    socklen_t sz = sizeof(struct sockaddr_in);
	int r = getsockname (theSocket,(struct sockaddr *)&socket_addr, &sz);
	if(r == 0) {
		thePort = ntohs(socket_addr.sin_port);
	}
	else
	{
		arts_warning("MCOP TCPServer: getsockname failed");
        close(theSocket);
		return false;
	}

	if(listen(theSocket,16) < 0)
	{
		arts_warning("MCOP TCPServer: can't listen on the socket");
        close(theSocket);
		return false;
	}

	arts_debug("TCPServer: listening on '%s'", url().c_str());
	return true;
}

string TCPServer::url()
{
	char xport[200];
	sprintf(xport,"%d",thePort);

	return "tcp:"+MCOPUtils::getFullHostname()+":"+xport;
}

void TCPServer::notifyIO(int fd, int types)
{
	assert(fd == theSocket);
	assert(socketOk);

	arts_debug("TCPManager: got notifyIO");
	if(types & IOType::read)
	{
		int clientfd;
		struct sockaddr_in incoming;
		socklen_t size_in = sizeof(struct sockaddr_in);

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
