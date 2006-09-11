/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *                2000 Stephan Kulow <coolo@kde.org>
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
#include <QtDBus/QtDBus>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <ktempfile.h>
#include <kprocess.h>
#include <klibloader.h>
#include <ktoolinvocation.h>
#include <klauncher_iface.h>

#include "kio/dataprotocol.h"
#include "kio/slave.h"
#include "kservice.h"
#include <kio/global.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>

#include <network/kserversocket.h>

using namespace KNetwork;

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
  };
}

void Slave::accept()
{
#ifndef Q_WS_WIN
    KStreamSocket *socket = serv->accept();
    slaveconn.init(socket);
#endif
    serv->deleteLater();
    serv = 0;
    slaveconn.connect(this, SLOT(gotInput()));
    unlinkSocket();
}

void Slave::unlinkSocket()
{
    if (m_socket.isEmpty()) return;
    QFile::remove( m_socket );
    m_socket.clear();
}

void Slave::timeout()
{
   if (!serv) return;
   kDebug(7002) << "slave failed to connect to application pid=" << m_pid << " protocol=" << m_protocol << endl;
   if (m_pid && (::kill(m_pid, 0) == 0))
   {
      int delta_t = (int) difftime(time(0), contact_started);
      kDebug(7002) << "slave is slow... pid=" << m_pid << " t=" << delta_t << endl;
      if (delta_t < SLAVE_CONNECTION_TIMEOUT_MAX)
      {
         QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, this, SLOT(timeout()));
         return;
      }
   }
   kDebug(7002) << "Houston, we lost our slave, pid=" << m_pid << endl;
   delete serv;
   serv = 0;
   unlinkSocket();
   dead = true;
   QString arg = m_protocol;
   if (!m_host.isEmpty())
      arg += "://"+m_host;
   kDebug(7002) << "slave died pid = " << m_pid << endl;
   ref();
   // Tell the job about the problem.
   emit error(ERR_SLAVE_DIED, arg);
   // Tell the scheduler about the problem.
   emit slaveDied(this);
   // After the above signal we're dead!!
   deref();
}

Slave::Slave(KServerSocket *socket, const QString &protocol, const QString &socketname)
  : SlaveInterface(&slaveconn), serv(socket), contacted(false)
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
    serv->setAcceptBuffered(false);
    connect(serv, SIGNAL(readyAccept()),
	    SLOT(accept() ) );
#endif
}

Slave::~Slave()
{
    // kDebug(7002) << "destructing slave object pid = " << m_pid << endl;
    if (serv != 0) {
        delete serv;
        serv = 0;
    }
    unlinkSocket();
    m_pid = 99999;
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

void Slave::hold(const KUrl &url)
{
   ref();
   {
      QByteArray data;
      QDataStream stream( &data, QIODevice::WriteOnly );
      stream << url;
      slaveconn.send( CMD_SLAVE_HOLD, data );
      slaveconn.close();
      dead = true;
      emit slaveDied(this);
   }
   deref();
   // Call KLauncher::waitForSlave(pid);
   {
      KToolInvocation::klauncher()->waitForSlave(m_pid);
   }
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

void Slave::send(int cmd, const QByteArray &arr) {
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
        kDebug(7002) << "slave died pid = " << m_pid << endl;
        // Tell the job about the problem.
        emit error(ERR_SLAVE_DIED, arg);
        // Tell the scheduler about the problem.
        emit slaveDied(this);
    }
    deref();
    // Here we might be dead!!
}

void Slave::kill()
{
    dead = true; // OO can be such simple.
    kDebug(7002) << "killing slave pid=" << m_pid << " (" << m_protocol << "://"
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
    QDataStream stream( &data, QIODevice::WriteOnly );
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
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << config;
    slaveconn.send( CMD_CONFIG, data );
}

Slave* Slave::createSlave( const QString &protocol, const KUrl& url, int& error, QString& error_text )
{
    //kDebug(7002) << "createSlave '" << protocol << "' for " << url.prettyUrl() << endl;
    // Firstly take into account all special slaves
    if (protocol == "data")
        return new DataProtocol();

    QString prefix = KStandardDirs::locateLocal("socket", KGlobal::instance()->instanceName());
    KTempFile socketfile(prefix, QLatin1String(".slave-socket"));
    if ( socketfile.status() != 0 )
    {
	error_text = i18n("Unable to create io-slave: %1", strerror(errno));
	error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
	return 0;
    }

    QString sockname = socketfile.name();

#ifdef __CYGWIN__
   socketfile.close();
#endif
   socketfile.unlink(); // can't bind if there is such a file

#ifndef Q_WS_WIN
    KServerSocket *kss = new KServerSocket(QFile::encodeName(sockname));
    kss->setFamily(KResolver::LocalFamily);
    kss->listen();

    Slave *slave = new Slave(kss, protocol, sockname);
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
    static bool bForkSlaves = getenv("KDE_FORK_SLAVES");

#ifdef Q_OS_UNIX
    if (!bForkSlaves)
    {
       // check the UID of klauncher
       QDBusReply<uint> reply = QDBusConnection::sessionBus().interface()->serviceUid(KToolInvocation::klauncher()->service());
       if (reply.isValid() && getuid() != reply)
          bForkSlaves = true;
    }
#endif

    if (bForkSlaves)
    {
       QString _name = KProtocolInfo::exec(protocol);
       if (_name.isEmpty())
       {
          error_text = i18n("Unknown protocol '%1'.", protocol);
          error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
          delete slave;
          return 0;
       }
       QString lib_path = KLibLoader::findLibrary(QFile::encodeName(_name));
       if (lib_path.isEmpty())
       {
          error_text = i18n("Can not find io-slave for protocol '%1'.", protocol);
          error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
          return 0;
       }

       KProcess proc;

       proc << KStandardDirs::locate("exe", "kioslave") << lib_path << protocol << "" << sockname;
       kDebug() << "kioslave" << ", " << lib_path << ", " << protocol << ", " << QString() << ", " << sockname << endl;

       proc.start(KProcess::DontCare);

#ifndef Q_WS_WIN
       slave->setPID(proc.pid());
       QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, slave, SLOT(timeout()));
#endif
       return slave;
    }

    org::kde::KLauncher* klauncher = KToolInvocation::klauncher();
    QString errorStr;
    QDBusReply<int> reply = klauncher->requestSlave(protocol, url.host(), sockname, errorStr);
    if (!reply.isValid()) {
	error_text = i18n("Cannot talk to klauncher: %1", klauncher->lastError().message() );
	error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete slave;
        return 0;
    }
    pid_t pid = reply;
    if (!pid)
    {
        error_text = i18n("Unable to create io-slave:\nklauncher said: %1", errorStr);
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

Slave* Slave::holdSlave( const QString &protocol, const KUrl& url )
{
    //kDebug(7002) << "holdSlave '" << protocol << "' for " << url.prettyUrl() << endl;
    // Firstly take into account all special slaves
    if (protocol == "data")
        return 0;

    QString prefix = KStandardDirs::locateLocal("socket", KGlobal::instance()->instanceName());
    KTempFile socketfile(prefix, QLatin1String(".slave-socket"));
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

    QDBusReply<int> reply = KToolInvocation::klauncher()->requestHoldSlave(url.url(), socketfile.name());
    if (!reply.isValid()) {
        delete slave;
        return 0;
    }
    pid_t pid = reply;
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

#include "slave.moc"
