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
    kdDebug(7002) << "slave has connected to application" << endl;
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
    m_port = 0;
    connect(serv, SIGNAL(accepted( KSocket* )),
	    SLOT(accept(KSocket*) ) );
}

Slave::~Slave()
{
    //kdDebug(7002) << "destructing slave object pid = " << m_pid << endl;
    if (serv != 0) {
        delete serv;
        serv = 0;
    }
    m_pid = 99999;
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

void Slave::suspend()
{
   slaveconn.suspend();
}

void Slave::resume()
{
   slaveconn.resume();
}

bool Slave::suspended()
{
   return slaveconn.suspended();
}


void Slave::gotInput()
{
    if (!dispatch())
    {
        slaveconn.close();
        dead = true;
        QString arg = m_protocol;
        if (!m_host.isEmpty())
            arg += "://"+m_host;
        kdDebug(7002) << "slave died (1) pid = " << m_pid << endl;
        // Tell the job about the problem.
        emit error(ERR_SLAVE_DIED, arg);
        kdDebug(7002) << "slave died (2) pid = " << m_pid << endl;
        // Tell the scheduler about the problem.
        emit slaveDied(this);
        // After the above signal we're dead!!
    }
}

void Slave::gotAnswer()
{
    int cmd = 0;
    QByteArray data;
    bool ok = true;

    if (slaveconn.read( &cmd, data ) == -1)
	ok = false;

    kdDebug(7002) << "got answer " << cmd << endl;

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
    kdDebug(7002) << "killing slave (" << m_protocol << "://"
		  << m_host << ")" << endl;
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
    m_user = user;
    m_passwd = passwd;

    slaveconn.connect(this, SLOT(gotAnswer()));

    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    stream << m_host << m_port << m_user << m_passwd;
    slaveconn.send( CMD_HOST, data );
}


Slave* Slave::createSlave( const KURL& url, int& error, QString& error_text )
{
    kdDebug(7002) << "createSlave for " << url.url() << endl;

    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();

    KTempFile socketfile(QString::null, QString::fromLatin1(".slave-socket"));

    KServerSocket *kss = new KServerSocket(socketfile.name().ascii());

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
	error_text = i18n("Unable to create io-slave:\nklauncher said: %1").arg(errorStr);
	error = KIO::ERR_INTERNAL;
        delete slave;
	return 0;
    }
    kdDebug(7002) << "PID of slave = " << pid << endl;
    slave->setPID(pid);

    return slave;
}

#include "slave.moc"
