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

#include "tcpconnection.h"
#include "dispatcher.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#define queue cqueue
#include <netdb.h>
#undef queue
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <errno.h>
#include "debug.h"

using namespace std;
using namespace Arts;

static struct sockaddr_in *parse_tcp_url(const char *url)
{
	static struct sockaddr_in addr;

	char *work = strdup(url);

	char *type = strtok(work,":");
	if(type == 0 || strcmp(type,"tcp") != 0) {
		free(work);
		return 0;
	}

	char *host = strtok(NULL,":");
	if(host == 0) {
		free(work);
		return 0;
	}

	char *port = strtok(NULL,":\n");
	if(port == 0) {
		free(work);
		return 0;
	}

	long portno = atol(port);
	if(portno < 1 || portno > 65535) {
		free(work);
		return 0;
	}

    struct hostent *server = gethostbyname(host);
   	if(server == 0)
	{
		arts_warning("parse_tcp_url: unknown host '%s'",host);
		free(work);
		return 0;
	}

	memset((void *) &addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *(u_long *)server->h_addr;
    addr.sin_port = htons(portno);

	free(work);
	return &addr;
}

static int tcp_connect(const char *url)
{
	struct sockaddr_in *remote_addr = parse_tcp_url(url);
	if(remote_addr == 0)
	{
		arts_warning("tcp_connect: couldn't parse url %s",url);
		return -1;
	}

	int my_socket = socket(AF_INET,SOCK_STREAM,0);
	if(my_socket < 0)
	{
		arts_warning("tcp_connect: unable to open socket for read");
		return -1;
	}

	struct linger lin;
    lin.l_onoff=1;      /* block a closing socket for 1 second */
    lin.l_linger=100;   /* if data is waiting to be sent */
    if ( setsockopt( my_socket, SOL_SOCKET, SO_LINGER,
                     (char*)&lin, sizeof(struct linger) ) < 0 )
    {
        arts_warning("tcp_connect: unable to set socket linger value to %d",
                lin.l_linger);
		close(my_socket);
        return -1;
    }

#ifdef TCP_NODELAY
	// enable TCP sending without nagle algorithm - this sends out requests
	// faster, because  they are not queued in the hope that more data will
	// need to be sent soon
	int on = 1;
	if(setsockopt(my_socket, IPPROTO_TCP, TCP_NODELAY, 
	              (char *)&on, sizeof(on)) < 0)
	{
		arts_debug("couldn't set TCP_NODELAY on socket %d\n", my_socket);
	}
#endif

	int rc;
	rc=connect(my_socket,(struct sockaddr *)remote_addr, sizeof(*remote_addr));
	if(rc != 0)
	{
		arts_warning("tcp_connect: can't connect to server (%s)", url);
		close(my_socket);
		return -1;
	}

	return my_socket;
}

TCPConnection::TCPConnection(string url)
{
	fd = tcp_connect(url.c_str());
	_broken = (fd == -1);

	if(!_broken)
	{
		Dispatcher::the()->ioManager()->watchFD(fd,
									IOType::read|IOType::except|IOType::reentrant,this);
		initReceive();
	}
}
