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

#include "slave.h"

#include <config.h>

#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

#include <QtCore/QBool>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <ktemporaryfile.h>
#include <k3process.h>
#include <klibloader.h>
#include <ktoolinvocation.h>
#include <klauncher_iface.h>

#include "dataprotocol.h"
#include "kservice.h"
#include <kio/global.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>

#include <network/k3serversocket.h>

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
    public:
        SlavePrivate(KServerSocket *socket, const QString &protocol,
                const QString &socketname) :
            m_protocol(protocol),
            m_slaveProtocol(protocol),
            m_socket(socketname),
            serv(socket),
            m_pid(0),
            m_port(0),
            contacted(false),
            dead(false),
            contact_started(time(0)),
            m_refCount(1)
        {
        }
        ~SlavePrivate()
        {
            if (serv != 0) {
                delete serv;
                serv = 0;
            }
        }
        QString m_protocol;
        QString m_slaveProtocol;
        QString m_host;
        QString m_user;
        QString m_passwd;
        QString m_socket;
        KNetwork::KServerSocket *serv;
        pid_t m_pid;
        quint16 m_port;
        bool contacted;
        bool dead;
        time_t contact_started;
        time_t idle_since;
        int m_refCount;
  };
}

void Slave::accept()
{
    KStreamSocket *socket = d->serv->accept();
    slaveconn.init(socket);
    d->serv->deleteLater();
    d->serv = 0;
    slaveconn.connect(this, SLOT(gotInput()));
    unlinkSocket();
}

void Slave::unlinkSocket()
{
    if (d->m_socket.isEmpty()) return;
    QFile::remove( d->m_socket );
    d->m_socket.clear();
}

void Slave::timeout()
{
   if (!d->serv) return;
   kDebug(7002) << "slave failed to connect to application pid=" << d->m_pid << " protocol=" << d->m_protocol << endl;
   if (d->m_pid && (::kill(d->m_pid, 0) == 0))
   {
      int delta_t = (int) difftime(time(0), d->contact_started);
      kDebug(7002) << "slave is slow... pid=" << d->m_pid << " t=" << delta_t << endl;
      if (delta_t < SLAVE_CONNECTION_TIMEOUT_MAX)
      {
         QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, this, SLOT(timeout()));
         return;
      }
   }
   kDebug(7002) << "Houston, we lost our slave, pid=" << d->m_pid << endl;
   delete d->serv;
   d->serv = 0;
   unlinkSocket();
   d->dead = true;
   QString arg = d->m_protocol;
   if (!d->m_host.isEmpty())
      arg += "://"+d->m_host;
   kDebug(7002) << "slave died pid = " << d->m_pid << endl;
   ref();
   // Tell the job about the problem.
   emit error(ERR_SLAVE_DIED, arg);
   // Tell the scheduler about the problem.
   emit slaveDied(this);
   // After the above signal we're dead!!
   deref();
}

Slave::Slave(KServerSocket *socket, const QString &protocol, const QString &socketname)
  : SlaveInterface(&slaveconn),
    d(new SlavePrivate(socket, protocol, socketname))
{
    if (d->serv != 0) {
        d->serv->setAcceptBuffered(false);
        connect(d->serv, SIGNAL(readyAccept()),
	        SLOT(accept() ) );
    }
}

Slave::~Slave()
{
    // kDebug(7002) << "destructing slave object pid = " << d->m_pid << endl;
    unlinkSocket();
    delete d;
}

QString Slave::protocol()
{
    return d->m_protocol;
}

void Slave::setProtocol(const QString & protocol)
{
    d->m_protocol = protocol;
}

QString Slave::slaveProtocol()
{
    return d->m_slaveProtocol;
}

QString Slave::host()
{
    return d->m_host;
}

quint16 Slave::port()
{
    return d->m_port;
}

QString Slave::user()
{
    return d->m_user;
}

QString Slave::passwd()
{
    return d->m_passwd;
}

void Slave::setIdle()
{
    d->idle_since = time(0);
}

bool Slave::isConnected()
{
    return d->contacted;
}

void Slave::setConnected(bool c)
{
    d->contacted = c;
}

void Slave::ref()
{
    d->m_refCount++;
}

void Slave::deref()
{
    d->m_refCount--;
    if (!d->m_refCount)
        delete this;
}

time_t Slave::idleTime()
{
    return (time_t) difftime(time(0), d->idle_since);
}

void Slave::setPID(pid_t pid)
{
    d->m_pid = pid;
}

int Slave::slave_pid()
{
    return d->m_pid;
}

bool Slave::isAlive()
{
    return !d->dead;
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
      d->dead = true;
      emit slaveDied(this);
   }
   deref();
   // Call KLauncher::waitForSlave(pid);
   {
      KToolInvocation::klauncher()->waitForSlave(d->m_pid);
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
        d->dead = true;
        QString arg = d->m_protocol;
        if (!d->m_host.isEmpty())
            arg += "://"+d->m_host;
        kDebug(7002) << "slave died pid = " << d->m_pid << endl;
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
    d->dead = true; // OO can be such simple.
    kDebug(7002) << "killing slave pid=" << d->m_pid << " (" << d->m_protocol << "://"
		  << d->m_host << ")" << endl;
    if (d->m_pid)
    {
       ::kill(d->m_pid, SIGTERM);
    }
}

void Slave::setHost( const QString &host, quint16 port,
                     const QString &user, const QString &passwd)
{
    d->m_host = host;
    d->m_port = port;
    d->m_user = user;
    d->m_passwd = passwd;

    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << d->m_host << d->m_port << d->m_user << d->m_passwd;
    slaveconn.send( CMD_HOST, data );
}

void Slave::resetHost()
{
    d->m_host = "<reset>";
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
    kDebug(7002) << "createSlave '" << protocol << "' for " << url.prettyUrl() << endl;
    // Firstly take into account all special slaves
    if (protocol == "data")
        return new DataProtocol();
#ifdef Q_WS_WIN
    QString sockname = getenv("COMPUTERNAME");
#else
    QString prefix = KStandardDirs::locateLocal("socket", KGlobal::mainComponent().componentName());
    KTemporaryFile *socketfile = new KTemporaryFile();
    socketfile->setPrefix(prefix);
    socketfile->setSuffix(QLatin1String(".slave-socket"));
    if ( !socketfile->open() )
    {
        error_text = i18n("Unable to create io-slave: %1", strerror(errno));
        error = KIO::ERR_CANNOT_LAUNCH_PROCESS;
        delete socketfile;
        return 0;
    }

    QString sockname = socketfile->fileName();
    delete socketfile; // can't bind if there is such a file
#endif

    KServerSocket *kss = new KServerSocket(QFile::encodeName(sockname));
    kss->setFamily(KResolver::LocalFamily);
    kss->listen();
    Slave *slave = new Slave(kss, protocol, sockname);

    // WABA: if the dcopserver is running under another uid we don't ask
    // klauncher for a slave, because the slave might have that other uid
    // as well, which might either be a) undesired or b) make it impossible
    // for the slave to connect to the application.
    // In such case we start the slave via K3Process.
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

#ifdef Q_OS_UNIX
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

       K3Process proc;

       proc << KStandardDirs::locate("exe", "kioslave") << lib_path << protocol << "" << sockname;
       kDebug() << "kioslave" << ", " << lib_path << ", " << protocol << ", " << QString() << ", " << sockname << endl;

       proc.start(K3Process::DontCare);

#ifndef Q_WS_WIN
       slave->setPID(proc.pid());
       QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, slave, SLOT(timeout()));
#endif
       return slave;
    }
#endif
    org::kde::KLauncher* klauncher = KToolInvocation::klauncher();
    QString errorStr;
    //qDebug() << __FUNCTION__ << protocol  << " " << url.host() << " " << sockname;
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
    slave->setPID(pid);
    QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, slave, SLOT(timeout()));
    return slave;
}

Slave* Slave::holdSlave( const QString &protocol, const KUrl& url )
{
    //kDebug(7002) << "holdSlave '" << protocol << "' for " << url.prettyUrl() << endl;
    // Firstly take into account all special slaves
    if (protocol == "data")
        return 0;
#ifdef Q_WS_WIN
    // localhost could not resolved yet, this s a bug in kdecore network resolver stuff
    // autoselect free tcp port
    KServerSocket *kss = new KServerSocket(getenv("COMPUTERNAME"),"0");
    QString sockname = kss->localAddress().serviceName();
#else
    QString prefix = KStandardDirs::locateLocal("socket", KGlobal::mainComponent().componentName());
    KTemporaryFile *socketfile = new KTemporaryFile();
    socketfile->setPrefix(prefix);
    socketfile->setSuffix(QLatin1String(".slave-socket"));
    if ( !socketfile->open() ) {
        delete socketfile;
        return 0;
    }

    QString sockname = socketfile->fileName();
    delete socketfile; // can't bind if there is such a file

    KServerSocket *kss = new KServerSocket(QFile::encodeName(sockname));
#endif
    Slave *slave = new Slave(kss, protocol, sockname);
    QDBusReply<int> reply = KToolInvocation::klauncher()->requestHoldSlave(url.url(), sockname);
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
    slave->setPID(pid);
    QTimer::singleShot(1000*SLAVE_CONNECTION_TIMEOUT_MIN, slave, SLOT(timeout()));
    return slave;
}

#include "slave.moc"
