/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *                2000 Stephan Kulow <coolo@kde.org>
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

#include <config.h>

#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

#include <qfile.h>

#include <dcopclient.h>
#include <kdebug.h>
#include <klocale.h>
#include <kapp.h>
#include <ktempfile.h>
#include <ksock.h>

#include "kio/slave.h"
#include "kio/kservice.h"
#include <kio/global.h>


#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

using namespace KIO;

void Slave::accept(KSocket *socket)
{
    kdDebug() << "slave has connected to application" << endl;
    slaveconn.init(socket);
    delete serv;
    serv = 0;
    QCString filename = QFile::encodeName(m_socket);
    unlink(filename.data());
    m_socket = QString::null;
}

Slave::Slave(KServerSocket *socket, const QString &protocol, const QString &socketname)
  : SlaveInterface(&slaveconn), serv(socket), contacted(false)
{
    m_protocol = protocol;
    m_socket = socketname;
    dead = false;
    contact_started = time(0);
    idle_since = contact_started;
    m_pid = 0;
    connect(serv, SIGNAL(accepted( KSocket* )),
	    SLOT(accept(KSocket*) ) );
}

void Slave::setIdle()
{
    idle_since = time(0);
}

time_t Slave::idleTime()
{
    return (time_t) difftime(time(0), idle_since);
}

void Slave::setPID(pid_t pid)
{
    m_pid = pid;
}

void Slave::gotInput()
{
    if (!dispatch())
    {
        QString arg = m_protocol;
        if (!m_host.isEmpty())
            arg += "://"+m_host;
        emit error(ERR_SLAVE_DIED, arg);
        emit slaveDied(this);
    }
}

void Slave::gotAnswer()
{
    int cmd = 0;
    QByteArray data;
    bool ok = true;

    if (slaveconn.read( &cmd, data ) == -1)
	ok = false;

    kdDebug() << "got answer " << cmd << endl;

    if (ok)
    {
	if (cmd == MSG_CONNECTED)
	    emit connected();
	else
	    dispatch(cmd, data);
        slaveconn.connect(this, SLOT(gotInput()));
    }
    else
    {
        slaveconn.close();
        // TODO: Report start up error to someone who is interested
        dead = true;
    }
}

void Slave::kill()
{
    dead = true; // OO can be such simple.
    kdDebug() << "killing slave (" << debugString(m_protocol) << ":\\" << debugString(m_host) << ")" << endl;
    if (m_pid)
    {
       ::kill(m_pid, SIGTERM);
    }
}

void Slave::setHost( const QString &host, int port,
                     const QString &user, const QString &passwd)
{
    m_host = host;
    m_port = port;
    m_user = user; // Why was this missing ? (David)
    m_passwd = passwd;

    slaveconn.connect(this, SLOT(gotAnswer()));

    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    stream << m_host << m_port << m_user << m_passwd;
    slaveconn.send( CMD_HOST, data );
}


Slave* Slave::createSlave( const KURL& url, int& error, QString& error_text )
{
    kdDebug() << "createSlave for " << debugString(url.url()) << endl;

    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();

#if 0
    // Check kioslave is running
    if (!client->isApplicationRegistered( "kioslave" ))
    {
        kdDebug() << "Trying to start kioslave" << endl;
        // Launch the kioslave service
        QString errort;
        QCString dcopName;
        if (KApplication::startServiceByDesktopName( "kioslave",
		QString::null, dcopName, errort))
        {
	   error = KIO::ERR_INTERNAL;
	   error_text=i18n("Can't launch kioslave: '%1'").arg(errort);
	   kdError() << error_text << endl;
           return 0;
        }
        if (dcopName != "kioslave")
        {
	   error = KIO::ERR_INTERNAL;
	   error_text = i18n("Error launching kioslave: got '%1' but expected 'kioslave'")
		.arg(dcopName);
           return 0;
        }
    }
#endif

    KTempFile socketfile(QString::null, QString::fromLatin1(".slave-socket"));

    KServerSocket *kss = new KServerSocket(socketfile.name());

    Slave *slave = new Slave(kss, url.protocol(), socketfile.name());

    QByteArray params, reply;
    QCString replyType;
    QDataStream stream(params, IO_WriteOnly);
    stream << url.protocol() << url.host() << socketfile.name();

    QCString launcher = KApplication::launcher();
    if (!client->call(launcher, launcher, "requestSlave(QString,QString,QString)",
	    params, replyType, reply)) {
	error_text = i18n("can't talk to klauncher");
	error = KIO::ERR_INTERNAL;
        delete slave;
	return 0;
    }
    QDataStream stream2(reply, IO_ReadOnly);
    QString errorStr;
    pid_t pid;
    stream2 >> pid >> errorStr;
    if (!pid)
    {
	error_text = i18n("Unable to create io-slave:\n%1").arg(errorStr);
	error = KIO::ERR_INTERNAL;
        delete slave;
	return 0;
    }
    kdDebug() << "PID of slave = " << pid << endl;
    slave->setPID(pid);

    return slave;
}

#include "slave.moc"
