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

#include "unixconnection.h"
#include "dispatcher.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <errno.h>

static struct sockaddr_un *parse_unix_url(const char *url)
{
	static struct sockaddr_un addr;

	char *work = strdup(url);

	char *type = strtok(work,":");
	if(type == 0 || strcmp(type,"unix") != 0) return 0;

	char *path = strtok(NULL,":\n");
	if(path == 0) return 0;

    memset((void *) &addr, '\0', sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path,path,108);
	addr.sun_path[107] = 0;
	return &addr;
}

static int unix_connect(const char *url)
{
	struct sockaddr_un *remote_addr = parse_unix_url(url);
	if(remote_addr == 0)
	{
		fprintf(stderr,"couldn't parse url %s\n",url);
		return 0;
	}

	int my_socket = socket(AF_UNIX,SOCK_STREAM,0);
	if(my_socket < 0)
	{
		fprintf(stderr,"unable to open socket for read");                     
		return 0;
	}

	struct linger lin;
    lin.l_onoff=1;      /* block a closing socket for 1 second */
    lin.l_linger=100;   /* if data is waiting to be sent */
    if ( setsockopt( my_socket, SOL_SOCKET, SO_LINGER,
                     (char*)&lin, sizeof(struct linger) ) < 0 )
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

UnixConnection::UnixConnection(string url)
{
	fd = unix_connect(url.c_str());
	_broken = (fd == 0);

	if(!_broken)
	{
		Dispatcher::the()->ioManager()->watchFD(fd,
									IOType::read|IOType::except,this);
		initReceive();
	}
}
