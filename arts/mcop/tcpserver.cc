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
#include "tcpserver.h"
#include "socketconnection.h"
#include "dispatcher.h"
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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

string xserverurl;

bool TCPServer::initSocket()
{
    struct sockaddr_in socket_addr;

	theSocket = socket(AF_INET,SOCK_STREAM,0);
	if(theSocket < 0)
	{
		fprintf(stderr,"MCOP TCPServer: can't create a socket\n");
		return false;
	}

	if(fcntl(theSocket,F_SETFL,O_NONBLOCK)<0)
	{
		fprintf(stderr,"MCOP TCPServer: can't initialize non blocking I/O\n");
		close(theSocket);
		return false;
	}

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons( 0 );		// chosse port freely
    socket_addr.sin_addr.s_addr = htonl( inet_addr("0.0.0.0") );
    if ( bind( theSocket, (struct sockaddr *) &socket_addr,
               sizeof(struct sockaddr_in) ) < 0 )
    {
		fprintf(stderr,"MCOP TCPServer: can't bind to port/address\n");
        close(theSocket);
		return false;
    }

    ksize_t sz = sizeof(sizeof(struct sockaddr_in));
	int r = getsockname (theSocket,(struct sockaddr *)&socket_addr, &sz);
	if(r == 0) {
		thePort = ntohs(socket_addr.sin_port);
	}
	else
	{
		fprintf(stderr,"MCOP TCPServer: getsockname failed\n");
        close(theSocket);
		return false;
	}

	if(listen(theSocket,16) < 0)
	{
		fprintf(stderr,"MCOP TCPServer: can't listen on the socket\n");
        close(theSocket);
		return false;
	}
	return &socket_addr;
}

string TCPServer::url()
{
	char xurl[200];
	sprintf(xurl,"tcp:localhost:%d",thePort);
	return xurl;
}

void TCPServer::notifyIO(int fd, int types)
{
	assert(fd == theSocket);
	assert(socketOk);

	printf("TCPManager: got notifyIO\n");
	if(types & IOType::read)
	{
		int clientfd;
		struct sockaddr_in incoming;
		ksize_t size_in = sizeof(struct sockaddr_in);

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
