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

#include "tcpconnection.h"
#include "dispatcher.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>

static struct sockaddr_in *parse_tcp_url(const char *url)
{
	static struct sockaddr_in addr;

	char *work = strdup(url);

	char *type = strtok(work,":");
	if(type == 0 || strcmp(type,"tcp") != 0) return 0;

	char *host = strtok(NULL,":");
	if(host == 0) return 0;

	char *port = strtok(NULL,":\n");
	if(port == 0) return 0;

	long portno = atol(port);
	if(portno < 1 || portno > 65535) return 0;

    struct hostent *server = gethostbyname(host);                                        
    memset((void *) &addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = *(u_long *)server->h_addr;
    addr.sin_port = htons(portno);

	return &addr;
}

static int tcp_connect(const char *url)
{
	struct sockaddr_in *remote_addr = parse_tcp_url(url);
	if(remote_addr == 0)
	{
		fprintf(stderr,"couldn't parse url %s\n",url);
		return 0;
	}

	int my_socket = socket(AF_INET,SOCK_STREAM,0);
	if(my_socket < 0)
	{
		fprintf(stderr,"unable to open socket for read");                     
		return 0;
	}

	struct linger lin;
    lin.l_onoff=1;      /* block a closing socket for 1 second */
    lin.l_linger=100;   /* if data is waiting to be sent */
    if ( setsockopt( my_socket, SOL_SOCKET, SO_LINGER,
                     &lin, sizeof(struct linger) ) < 0 )
    {
        fprintf(stderr,"Unable to set socket linger value to %d\n",
                lin.l_linger);
        return 0;
    }

	int rc;
	rc=connect(my_socket,(struct sockaddr *)remote_addr, sizeof(*remote_addr));
	if(rc != 0)
	{
		fprintf(stderr,"can't connect to server");
		return 0;
	}

	return my_socket;
}

TCPConnection::TCPConnection(string url)
{
	fd = tcp_connect(url.c_str());
	_broken = (fd == 0);

	if(!_broken)
	{
		Dispatcher::the()->ioManager()->watchFD(fd,
									IOType::read|IOType::except,this);
		initReceive();
	}
}
