/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>

#include <kdebug.h>
#include <ksock.h>

#include "kio/slavewrapper.h"
#include "kio/slavebase.h"
#include "kio/slaveinterface.h"

using namespace KIO;

SlaveWrapper::SlaveWrapper(SlaveBase *s, 
                           const QString &pool_socket, 
                           const QString &app_socket)
    : slave(s), mPoolSocket(pool_socket), mAppSocket(app_socket)
{
fprintf(stderr, "SlaveWrapper::SlaveWrapper(... , %s)\n", mAppSocket.ascii());
    mConnectedToApp = true;
    connectSlave(mAppSocket);
fprintf(stderr, "SlaveWrapper: Connected!\n");
}

SlaveWrapper::~SlaveWrapper()
{
    delete slave;
}

void SlaveWrapper::dispatchLoop()
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    while (true) {
	FD_ZERO(&rfds);

	if (appconn.inited())
	  FD_SET(appconn.fd_from(), &rfds);

	/* Wait up to 30 seconds. */
	tv.tv_sec = 30;
	tv.tv_usec = 0;

	retval = select(appconn.fd_from()+ 1, &rfds, NULL, NULL, &tv);
	/* Don't rely on the value of tv now! */

	if (retval > 0) 
        {
	    if (FD_ISSET(appconn.fd_from(), &rfds)) 
            { // dispatch application messages
		int cmd;
		QByteArray data;
                if ( appconn.read(&cmd, data) != -1 )
                {
		  kDebugInfo(7019, "app said %c", cmd);
                  if (cmd == CMD_SLAVE_CONNECT)
                  {
                     QString app_socket;
                     QDataStream stream( data, IO_ReadOnly);
                     stream >> app_socket;
                     kDebugInfo(7019, "slavewrapper: Connecting to new app (%s).", app_socket.ascii());
                     appconn.send( MSG_SLAVE_ACK );
                     disconnectSlave();
                     mConnectedToApp = true;
                     connectSlave(app_socket);
                  }
                  else
                  {
                     slave->dispatch(cmd, data);
                  }
                } 
                else // some error occured, perhaps no more application
                {
// When the app exits, should the slave be put back in the pool ?
                  if (mConnectedToApp)
                  {
                     kDebugInfo(7019, "slavewrapper: Communication with app lost. Returning to slave pool.");
                     disconnectSlave();
                     mConnectedToApp = false;
                     connectSlave(mPoolSocket);
                  }
                  else
                  {
                     kDebugInfo(7019, "slavewrapper: Communication with pool lost. Exiting.");
                     exit(0);
                  }
                }
	    }
	} 
        else if (retval == -1) // error
        {
          kDebugInfo(7019, "slavewrapper: select returned error %s (%d)", errno==EBADF?"EBADF":errno==EINTR?"EINTR":errno==EINVAL?"EINVAL":errno==ENOMEM?"ENOMEM":"unknown",errno);
          exit(0);
        }
    }
}

void SlaveWrapper::connectSlave(const QString& path)
{
    appconn.init(new KSocket(path));
    slave->setConnection(&appconn);
}

void SlaveWrapper::disconnectSlave()
{
    appconn.close();
}

