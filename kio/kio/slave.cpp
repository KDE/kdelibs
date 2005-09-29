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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
#include <qtimer.h>

#include <dcopclient.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <ktempfile.h>
#include <ksock.h>
#include <kprocess.h>
#include <klibloader.h>

#include "kio/dataprotocol.h"
#include "kio/slave.h"
#include "kio/kservice.h"
#include <kio/global.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp"
#endif

using namespace KIO;

#define SLAVE_CONNECTION_TIMEOUT_MIN	   2

// Without debug info we consider it an error if the slave doesn't connect
// within 10 seconds.
// With debug info we give the slave an hour so that developers have a chance
// to debug their slave.
#ifdef NDEBUG
#define SLAVE_CONNECTION_TIMEOUT_MAX      10
#else
#define SLAVE_CONNECTION_TIMEOUT_MAX    3600
#endif

namespace KIO {

  /**
   * @internal
   */
  class SlavePrivate {
  public:
    bool derived;	// true if this instance of Slave is actually an
    			// instance of a derived class.

    SlavePrivate(bool derived) : derived(derived) {}
  };
}

void Slave::accept(KSocket *socket)
{
#ifndef Q_WS_WIN
    slaveconn.init(socket);
#endif
    delete serv;
    serv = 0;
    slaveconn.connect(this, SLOT(gotInput()));
    unlinkSocket();
}

void Slave::unlinkSocket()
{
    if (m_socket.isEmpty()) return;
    QCString filename = QFile::encodeName(m_socket);
    unlink(filename.data());
    m_socket = QString::null;
}

void Slave::timeout()
{
   if (!serv) return;
   kdDebug(7002) << "slave failed to connect to application pid=" << m_pid << " protocol=" << m_protocol << endl;
   if (m_pid && (::kill(m_pid, 0) == 0))
   {
      int delta_t = (int) difftime(time(0), contact_started);
      kdDebug(7002) << "slave is slow... pid=" << m_pid << " t=" << delta_t << endl;
      if (delta_t < SLAVE_CONNECTION_TIMEOUT_MAX)
      {
         QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, this, SLOT(timeout()));
         return;
      }
   }
   kdDebug(7002) << "Houston, we lost our slave, pid=" << m_pid << endl;
   delete serv;
   serv = 0;
   unlinkSocket();
   dead = true;
   QString arg = m_protocol;
   if (!m_host.isEmpty())
      arg += "://"+m_host;
   kdDebug(7002) << "slave died pid = " << m_pid << endl;
   ref();
   // Tell the job about the problem.
   emit error(ERR_SLAVE_DIED, arg);
   // Tell the scheduler about the problem.
   emit slaveDied(this);
   // After the above signal we're dead!!
   deref();
}

Slave::Slave(KServerSocket *socket, const QString &protocol, const QString &socketname)
  : SlaveInterface(&slaveconn), serv(socket), contacted(false),
  	d(new SlavePrivate(false))
{
    m_refCount = 1;
    m_protocol = protocol;
    m_slaveProtocol = protocol;
    m_socket = socketname;
    dead = false;
    contact_started = time(0);
    idle_since = contact_started;
    m_pid = 0;
    m_port = 0;
#ifndef Q_WS_WIN
    connect(serv, SIGNAL(accepted( KSocket* )),
	    SLOT(accept(KSocket*) ) );
#endif
}

Slave::Slave(bool /*derived*/, KServerSocket *socket, const QString &protocol,
	const QString &socketname)
  : SlaveInterface(&slaveconn), serv(socket), contacted(false),
  	d(new SlavePrivate(true))
{
    // FIXME: hmm, duplicating code here from public ctor, no good (LS)
    m_refCount = 1;
    m_protocol = protocol;
    m_slaveProtocol = protocol;
    m_socket = socketname;
    dead = false;
    contact_started = time(0);
    idle_since = contact_started;
    m_pid = 0;
    m_port = 0;
    if (serv != 0) {
#ifndef Q_WS_WIN
      connect(serv, SIGNAL(accepted( KSocket* )),
        SLOT(accept(KSocket*) ) );
#endif
    }
}

Slave::~Slave()
{
    // kdDebug(7002) << "destructing slave object pid = " << m_pid << endl;
    if (serv != 0) {
        delete serv;
        serv = 0;
    }
    unlinkSocket();
    m_pid = 99999;
    delete d;
    d = 0;
}

void Slave::setProtocol(const QString & protocol)
{
    m_protocol = protocol;
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

void Slave::hold(const KURL &url)
{
   if (d->derived) {		// TODO: clean up before KDE 4
     HoldParams params;
     params.url = &url;
     virtual_hook(VIRTUAL_HOLD, &params);
     return;
   }/*end if*/

   ref();
   {
      QByteArray data;
      QDataStream stream( data, IO_WriteOnly );
      stream << url;
      slaveconn.send( CMD_SLAVE_HOLD, data );
      slaveconn.close();
      dead = true;
      emit slaveDied(this);
   }
   deref();
   // Call KLauncher::waitForSlave(pid);
   {
      DCOPClient *client = kapp->dcopClient();
      if (!client->isAttached())
         client->attach();

      QByteArray params, reply;
      QCString replyType;
      QDataStream stream(params, IO_WriteOnly);
      pid_t pid = m_pid;
      stream << pid;

      QCString launcher = KApplication::launcher();
      client->call(launcher, launcher, "waitForSlave(pid_t)",
	    params, replyType, reply);
   }
}

void Slave::suspend()
{
   if (d->derived) {		// TODO: clean up before KDE 4
     virtual_hook(VIRTUAL_SUSPEND, 0);
     return;
   }/*end if*/

   slaveconn.suspend();
}

void Slave::resume()
{
   if (d->derived) {		// TODO: clean up before KDE 4
     virtual_hook(VIRTUAL_RESUME, 0);
     return;
   }/*end if*/

   slaveconn.resume();
}

bool Slave::suspended()
{
   if (d->derived) {		// TODO: clean up before KDE 4
     SuspendedParams params;
     virtual_hook(VIRTUAL_SUSPENDED, &params);
     return params.retval;
   }/*end if*/

   return slaveconn.suspended();
}

void Slave::send(int cmd, const QByteArray &arr) {
   if (d->derived) {		// TODO: clean up before KDE 4
     SendParams params;
     params.cmd = cmd;
     params.arr = &arr;
     virtual_hook(VIRTUAL_SEND, &params);
     return;
   }/*end if*/

   slaveconn.send(cmd, arr);
}

void Slave::gotInput()
{
    ref();
    if (!dispatch())
    {
        slaveconn.close();
        dead = true;
        QString arg = m_protocol;
        if (!m_host.isEmpty())
            arg += "://"+m_host;
        kdDebug(7002) << "slave died pid = " << m_pid << endl;
        // Tell the job about the problem.
        emit error(ERR_SLAVE_DIED, arg);
        // Tell the scheduler about the problem.
        emit slaveDied(this);
        // After the above signal we're dead!!
    }
    deref();
}

void Slave::kill()
{
    dead = true; // OO can be such simple.
    kdDebug(7002) << "killing slave pid=" << m_pid << " (" << m_protocol << "://"
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

    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    stream << m_host << m_port << m_user << m_passwd;
    slaveconn.send( CMD_HOST, data );
}

void Slave::resetHost()
{
    m_host = "<reset>";
}

void Slave::setConfig(const MetaData &config)
{
    QByteArray data;
    QDataStream stream( data, IO_WriteOnly );
    stream << config;
    slaveconn.send( CMD_CONFIG, data );
}

Slave* Slave::createSlave( const QString &protocol, const KURL& url, int& error, QString& error_text )
{
    //kdDebug(7002) << "createSlave '" << protocol << "' for " << url.prettyURL() << endl;
    // Firstly take into account all special slaves
    if (protocol == "data")
        return new DataProtocol();

    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();

    QString prefix = locateLocal("socket", KGlobal::instance()->instanceName());
    KTempFile socketfile(prefix, QString::fromLatin1(".slave-socket"));
    if ( socketfile.status() != 0 )
    {
	error_text = i18n("Unable to create io-slave: %1").arg(strerror(errno));
	error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
	return 0;
    }

#ifdef __CYGWIN__
   socketfile.close();
#endif
    
#ifndef Q_WS_WIN
    KServerSocket *kss = new KServerSocket(QFile::encodeName(socketfile.name()));

    Slave *slave = new Slave(kss, protocol, socketfile.name());
#else
    Slave *slave = 0;
#endif

    // WABA: if the dcopserver is running under another uid we don't ask
    // klauncher for a slave, because the slave might have that other uid
    // as well, which might either be a) undesired or b) make it impossible
    // for the slave to connect to the application.
    // In such case we start the slave via KProcess.
    // It's possible to force this by setting the env. variable
    // KDE_FORK_SLAVES, Clearcase seems to require this.
    static bool bForkSlaves = !QCString(getenv("KDE_FORK_SLAVES")).isEmpty();
    
    if (bForkSlaves || !client->isAttached() || client->isAttachedToForeignServer())
    {
       QString _name = KProtocolInfo::exec(protocol);
       if (_name.isEmpty())
       {
          error_text = i18n("Unknown protocol '%1'.").arg(protocol);
          error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
          delete slave;
          return 0;
       }
       QString lib_path = KLibLoader::findLibrary(_name.latin1());
       if (lib_path.isEmpty())
       {
          error_text = i18n("Can not find io-slave for protocol '%1'.").arg(protocol);
          error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
          return 0;
       }

       KProcess proc;

       proc << locate("exe", "kioslave") << lib_path << protocol << "" << socketfile.name();
       kdDebug(7002) << "kioslave" << ", " << lib_path << ", " << protocol << ", " << QString::null << ", " << socketfile.name() << endl;

       proc.start(KProcess::DontCare);

#ifndef Q_WS_WIN
       slave->setPID(proc.pid());
       QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, slave, SLOT(timeout()));
#endif
       return slave;
    }


    QByteArray params, reply;
    QCString replyType;
    QDataStream stream(params, IO_WriteOnly);
    stream << protocol << url.host() << socketfile.name();

    QCString launcher = KApplication::launcher();
    if (!client->call(launcher, launcher, "requestSlave(QString,QString,QString)",
	    params, replyType, reply)) {
	error_text = i18n("Cannot talk to klauncher");
	error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
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
        error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete slave;
        return 0;
    }
#ifndef Q_WS_WIN
    slave->setPID(pid);
    QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, slave, SLOT(timeout()));
#endif
    return slave;
}

Slave* Slave::holdSlave( const QString &protocol, const KURL& url )
{
    //kdDebug(7002) << "holdSlave '" << protocol << "' for " << url.prettyURL() << endl;
    // Firstly take into account all special slaves
    if (protocol == "data")
        return 0;

    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();

    QString prefix = locateLocal("socket", KGlobal::instance()->instanceName());
    KTempFile socketfile(prefix, QString::fromLatin1(".slave-socket"));
    if ( socketfile.status() != 0 )
	return 0;

#ifdef __CYGWIN__
   socketfile.close();
   socketfile.unlink();
#endif

#ifndef Q_WS_WIN
    KServerSocket *kss = new KServerSocket(QFile::encodeName(socketfile.name()));

    Slave *slave = new Slave(kss, protocol, socketfile.name());
#else
    Slave *slave = 0;
#endif

    QByteArray params, reply;
    QCString replyType;
    QDataStream stream(params, IO_WriteOnly);
    stream << url << socketfile.name();

    QCString launcher = KApplication::launcher();
    if (!client->call(launcher, launcher, "requestHoldSlave(KURL,QString)",
        params, replyType, reply)) {
        delete slave;
        return 0;
    }
    QDataStream stream2(reply, IO_ReadOnly);
    pid_t pid;
    stream2 >> pid;
    if (!pid)
    {
        delete slave;
        return 0;
    }
#ifndef Q_WS_WIN
    slave->setPID(pid);
    QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, slave, SLOT(timeout()));
#endif
    return slave;
}

void Slave::virtual_hook( int id, void* data ) {
  KIO::SlaveInterface::virtual_hook( id, data );
}

#include "slave.moc"
