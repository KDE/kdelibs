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

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <ltdl.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <qfile.h>

#include <dcopclient.h>
#include <kdebug.h>
#include <ksock.h>
#include <klibloader.h>
#include <kstddirs.h>
#include <ktempfile.h>
#include <klocale.h>

#include "kio/global.h"
#include "kio/connection.h"
#include "slavewrapper.h"
#include "kio/slaveinterface.h"

#include <X11/Xlib.h>

#include "kioslave.h"

using namespace KIO;

IdleSlave::IdleSlave(KSocket *socket)
{
   mConn.init(socket);
   mConn.connect(this, SLOT(gotInput()));
   mConn.send( CMD_SLAVE_STATUS );
   mPid = 0;
   // Timeout!
}

void
IdleSlave::gotInput()
{
   int cmd;
   QByteArray data;
   if (mConn.read( &cmd, data) == -1)
   {
      // Communication problem with slave. 
      kDebugInfo(7016, "SlavePool: No communication with slave.");
      delete this;
   }
   else if (cmd == MSG_SLAVE_ACK)
   {
      kDebugInfo(7016, "SlavePool: Slave is connecting to app.");
      delete this;
   }
   else if (cmd != MSG_SLAVE_STATUS)
   {
      kDebugInfo(7016, "SlavePool: Unexpected data from slave.");
      delete this;
   }
   else 
   {   
      QDataStream stream( data, IO_ReadOnly );
      pid_t pid;
      QCString protocol;
      QString host;
      Q_INT8 b;
      stream >> pid >> protocol >> host >> b;
      mPid = pid;
      mConnected = (b != 0);
      mProtocol = protocol;
      mHost = host;
      kDebugInfo(7016, "SlavePool: SlaveStatus = %s %s %s",
           mProtocol.data(), mHost.ascii(), 
           mConnected ? "Connected" : "Not connected");
   }
}

void
IdleSlave::connect(const QString &app_socket)
{
   kDebugInfo(7016, "SlavePool: New mission for slave (%s %s %s)",
           mProtocol.data(), mHost.ascii(), 
           mConnected ? "Connected" : "Not connected");
   QByteArray data;
   QDataStream stream( data, IO_WriteOnly);
   stream << app_socket;
   mConn.send( CMD_SLAVE_CONNECT, data );
   // Timeout!
}

bool
IdleSlave::match(const QString &protocol, const QString &host, bool connected)
{
   if (protocol != mProtocol) return false;
   if (host.isEmpty()) return true;
   if (host != mHost) return false;
   if (!connected) return true;
   if (!mConnected) return false;
   return true;
}

KIODaemon::KIODaemon(int &argc, char **argv) :
    KUniqueApplication(argc, argv, "kioslave", false)
{
    KTempFile domainname(QString::null, QString::fromLatin1(".slave-socket"));
    mPoolSocketName = domainname.name();
    mPoolSocket = new KServerSocket(mPoolSocketName);
    connect(mPoolSocket, SIGNAL(accepted( KSocket *)),
            SLOT(acceptSlave(KSocket *)));
}

void
KIODaemon::acceptSlave(KSocket *slaveSocket)
{
    kDebugInfo(7016, "SlavePool: accepSlave(...)");
    IdleSlave *slave = new IdleSlave(slaveSocket);
    // Send it a SLAVE_STATUS command.
    mSlaveList.append(slave);
    connect(slave, SIGNAL(destroyed()), this, SLOT(slotSlaveGone()));
}

void
KIODaemon::slotSlaveGone()
{
    kDebugInfo(7016, "SlavePool: slotSlaveGone(...)");
    IdleSlave *slave = (IdleSlave *) sender();
    mSlaveList.removeRef(slave);
}

pid_t
KIODaemon::requestSlave(const QString &protocol, 
                        const QString &host,
                        const QString &app_socket, QString &error)
{
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
        if (slave->match(protocol, host, true))
           break;
    }
    if (!slave)
    {
       for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
       {
          if (slave->match(protocol, host, false))
             break;
       }
    }
    if (!slave)
    {
       for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
       {
          if (slave->match(protocol, QString::null, false))
             break;
       }
    }
    if (slave)
    {
       mSlaveList.removeRef(slave);
       slave->connect(app_socket);
       return slave->pid();
    }

    kDebugInfo(7016, "requestSlave( %s, %s, %s)",
		protocol.ascii(), host.ascii(), app_socket.ascii());
    QString protocol_library = QString::fromLatin1("%1/.libs/kio_%2.la").arg(protocol).arg(protocol);
    if (!QFile::exists(protocol_library))
	 protocol_library = locate("lib", QString("kio_%1.la").arg(protocol));

    if (protocol_library.isNull()) {
	error = i18n("error: no such protocol");
        return 0;
    }

    int fd[2];

    if (::socketpair(PF_UNIX, SOCK_STREAM, 0, fd) == -1) {
	perror("socketpair");
	error = i18n("error: pipe creation failed");
        return 0;
    }
    QApplication::flushX();

    int pid = fork();
    if (pid == -1) {
	perror("fork");
	error = i18n("error: fork failed");
        return 0;
    }
    static char errors[] = { 'K', // OK
			     'L' // Loading failed
    };
    const int ERR_OK = 0;
    const int ERR_LOADING = 1;

    if (pid == 0) { // Child

	::close(ConnectionNumber(qt_xdisplay())); // make sure noone makes X connections to us
	::close(fd[0]);

	lt_dlhandle handle = lt_dlopen( protocol_library );

	if ( !handle ) {
	    kDebugInfo(7016, "trying to load support for %s failed with %s", protocol.ascii(), lt_dlerror() );
	    ::write(fd[1], errors + ERR_LOADING, 1);
	    exit(0);
	}
	
	KLibrary *lib = new KLibrary( protocol, protocol_library, handle );

	QCString symname;
	symname.sprintf("init_%s", protocol.latin1() );

	void* sym = lib->symbol( symname );
	if ( !sym )
        {
	    kDebugInfo(7016, "KLibrary: The library does not offer a KDE compatible factory");
	    ::write(fd[1], errors + ERR_LOADING, 1);
	    exit(0);
        }

	typedef SlaveBase* (*t_func)();
	t_func func = (t_func)sym;
	SlaveBase *serv = func();
	
	if( !serv ) {
	    kDebugInfo(7016, "KLibrary: The library does not offer a KDE compatible factory");
	    ::write(fd[1], errors + ERR_LOADING, 1);
	    exit(0);
	}

	::write(fd[1], errors + ERR_OK, 1);
        ::close(fd[1]);
	
	SlaveWrapper *ksw = new SlaveWrapper(serv, mPoolSocketName, app_socket);
	ksw->dispatchLoop();

	exit(0);
	return 0; // never reached

    } else {
        ::close(fd[1]);

	// read the first character of the child to
	// see how it goes :/
	char result;
	if (::read(fd[0], &result, 1) != 1) {
	    perror("read");
            ::close(fd[0]);
	    error = i18n("error: io-slave crashed");
            return 0;
	}
        ::close(fd[0]);

	if (result != errors[ERR_OK]) {
	    error = i18n("error: loading failed");
            return 0;
	}

        // Success
	return pid;
    }
}

bool KIODaemon::process(const QCString &fun, const QByteArray &data,
			QCString &replyType, QByteArray &replyData)
{
    if (KUniqueApplication::process(fun, data, replyType, replyData))
	return true;

    QDataStream stream(data, IO_ReadOnly);
    QDataStream output(replyData, IO_WriteOnly);

    if (fun == "requestSlave(QString,QString,QString)") {
	QString protocol;
        QString host;
        QString app_socket;
	stream >> protocol >> host >> app_socket;
	replyType = "QString";
        QString error;
	pid_t pid = requestSlave(protocol, host, app_socket, error);
	output << pid << error;;
	return true;
    }
    fprintf(stderr, "Unknown function '%s'\n", fun.data());

    return false;
}

void slave_sigchld_handler(int)
{
    int status;
    /*pid = */waitpid(-1, &status, WNOHANG);
    signal(SIGCHLD, slave_sigchld_handler);
}

int main(int argc, char **argv)
{
  //  KCmdLineArgs::init(argc, argv, "kioslave",
  //  "a tool to start kio protocols", "0.0");

  // KUniqueApplication::addCmdLineOptions();

  if (!KUniqueApplication::start(argc, argv, "kioslave"))
    {
        fprintf(stderr, "kioslave already running!\n");
	return 0;
    }
  signal(SIGCHLD, slave_sigchld_handler);

    KIODaemon k(argc,argv);
    return k.exec(); // keep running
}

#include "kioslave.moc"
