/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, mywrite to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#include <qfile.h>
#include <QProcess>

#include <kconfig.h>
#include <kdebug.h>
#include <klibloader.h>
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>

#include "kio/global.h"
#include "kio/connection.h"
#include "kio/slaveinterface.h"

#include "klauncher.h"
#include "klauncher_cmds.h"
#include "klauncher_adaptor.h"
#include <windows.h>

#ifdef Q_WS_X11
#include <kstartupinfo.h>
#include <X11/Xlib.h>
#endif

QList<QProcess *>processList;

#ifdef Q_WS_WIN
/* note: 
   this is an initial version of klauncher for win32
   by replacing all kdeinit related calls (mywrite/myread) to a local handler 
   handle_request() which is mostly taken from the related parts of kinit.cpp 
   
   The current state is, that klauncher is registrated in dbus, request are processed 
   but process fails to start because of missing shared libraries. 
   It seems that spawnve does not search the full path environment var. May be it is 
   better to use QProcess or CreateProcess.    
   args and envs are not handled full. 
*/
klauncher_header response_header;
char response_data[1024];
   
int handle_request(klauncher_header request_header, char *request_data)
{
   if (request_header.cmd == LAUNCHER_OK)
   {
      return true;
   }
   else if (request_header.arg_length &&
      ((request_header.cmd == LAUNCHER_EXEC) ||
       (request_header.cmd == LAUNCHER_EXT_EXEC) ||
       (request_header.cmd == LAUNCHER_SHELL ) ||
       (request_header.cmd == LAUNCHER_KWRAPPER) ||
       (request_header.cmd == LAUNCHER_EXEC_NEW)))
   {
			QStringList arglist;
			QStringList envlist;
      pid_t pid;
//      klauncher_header response_header;
//      long response_data;
      long l;
      memcpy( &l, request_data, sizeof( long ));
      int argc = l;
      char *name = request_data + sizeof(long);
      char *args = name + strlen(name) + 1;
      const char *cwd = 0;
      int envc = 0;
      const char *envs = 0;
      const char *tty = 0;
      int avoid_loops = 0;
      const char *startup_id_str = "0";

//#ifndef NDEBUG
		int launcher;
     fprintf(stderr, "kdeinit: Got %s '%s' from %s.\n",
        (request_header.cmd == LAUNCHER_EXEC ? "EXEC" :
        (request_header.cmd == LAUNCHER_EXT_EXEC ? "EXT_EXEC" :
        (request_header.cmd == LAUNCHER_EXEC_NEW ? "EXEC_NEW" :
        (request_header.cmd == LAUNCHER_SHELL ? "SHELL" : "KWRAPPER" )))),
         name, launcher ? "launcher" : "socket" );
//#endif

      char *arg_n = args;
      for(int i = 1; i < argc; i++)
      {
				arglist << arg_n;
        arg_n = arg_n + strlen(arg_n) + 1;
      }

      if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER )
      {
         // Shell or kwrapper
         cwd = arg_n; arg_n += strlen(cwd) + 1;
      }
      if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
          || request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW )
      {
         memcpy( &l, arg_n, sizeof( long ));
         envc = l;
         arg_n += sizeof(long);
         envs = arg_n;
         for(int i = 0; i < envc; i++)
         {
           envlist << arg_n;
           arg_n = arg_n + strlen(arg_n) + 1;
         }
         if( request_header.cmd == LAUNCHER_KWRAPPER )
         {
             tty = arg_n;
             arg_n += strlen( tty ) + 1;
         }
      }

     if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
         || request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW )
     {
         memcpy( &l, arg_n, sizeof( long ));
         avoid_loops = l;
         arg_n += sizeof( long );
     }

     if( request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER
         || request_header.cmd == LAUNCHER_EXT_EXEC )
     {
         startup_id_str = arg_n;
         arg_n += strlen( startup_id_str ) + 1;
     }

     if ((request_header.arg_length > (arg_n - request_data)) &&
         (request_header.cmd == LAUNCHER_EXT_EXEC || request_header.cmd == LAUNCHER_EXEC_NEW ))
     {
         // Optional cwd
         cwd = arg_n; arg_n += strlen(cwd) + 1;
     }

     if ((arg_n - request_data) != request_header.arg_length)
     {
//#ifndef NDEBUG
       fprintf(stderr, "kdeinit: EXEC request has invalid format.\n");
//#endif
       return 0;
     }

			printf("argc %d, name %s, args %s, cwd %s, envc %s, envs %s\n",
      	argc, name, args, cwd, envc, envs);

/*
      char path[MAX_PATH];
      LPSTR lpFile;

		  if (!SearchPathA(NULL, name, ".exe", sizeof(path), path, &lpFile)) {
	      printf ("could not find %s\n",name);
	      return 0;
	    }
			myargs[0] = path;

			char *_envs[1];

			_envs[0] = 0;
			
			int handle = spawnve (P_WAIT, name,
                           myargs,
                           _envs//envs
                        	);      

			printf("handle %d",handle); 
*/
			QProcess *process  = new QProcess;
//			process.setEnvironment(envlist);
			process->start(name,arglist);
			QByteArray _stderr = process->readAllStandardError();
			QByteArray _stdout = process->readAllStandardOutput();
			printf("%s",_stdout.data());
			printf("%s",_stderr.data());

			_PROCESS_INFORMATION* _pid = process->pid();
			pid = _pid ? _pid->dwProcessId : 0;

			printf("pid = %d\n",pid);
/*
      pid = launch( argc, name, args, cwd, envc, envs,
          request_header.cmd == LAUNCHER_SHELL || request_header.cmd == LAUNCHER_KWRAPPER,
          tty, avoid_loops, startup_id_str );

*/   

    if (pid) {
			response_header.cmd = LAUNCHER_OK;
	    response_header.arg_length = sizeof(long);
	    *((long*)response_data) = pid;
	    printf("return pid\n");
		}
		else {
			char *error = "error occured";
			response_header.cmd = LAUNCHER_ERROR;
			response_header.arg_length = strlen(error)+1;
			strcpy(response_data,error);
	    printf("return error\n");
		}
		return 0;     
	}
}


int mywrite(int sock, void *p, int len)
{
	char *buf = (char *)p;
	printf("write len:%d data:\n",len);
	for (int i = 0; i < len; i++)
		printf("%02x '%c'\n",buf[i],buf[i] >= 0x20 ? buf[i] : ' ');
	printf("\n");

  char *request_data = 0;
	static klauncher_header request_header;
	static int waitforData = 0;
	if (!waitforData) {
	  request_header = *(klauncher_header*)p;
  	if ( request_header.arg_length != 0 )
  		waitforData = 1;
  	else {
  		char *data = "";
			handle_request(request_header,data);
			return 0;
		}
 	}
 	else {
  	waitforData = 0;
		handle_request(request_header,buf);
	}  
	return len;
}
int myread(int sock, void *buf, int len)
{
	static int sendData = 0;
  klauncher_header *request_header = (klauncher_header*)buf;
	printf("cmd=%d",response_header.cmd);
	
  if (response_header.cmd && sendData) {
	  memcpy(buf,response_data,response_header.arg_length);
	  sendData = 0;
	  response_header.cmd = 0;
		printf("read data len=%d return len=%d\n",len,response_header.arg_length);
		return sizeof(response_header.arg_length);
	} 
	else if (response_header.cmd) {
		printf("return header\n");
	  request_header->cmd = response_header.cmd;
	  request_header->arg_length = response_header.arg_length;
	  if (response_header.arg_length)
	  	sendData = 1;
		printf("read header len=%d return len=%d\n",len,sizeof(*request_header));
		return sizeof(*request_header);
  }
	else {
	  request_header->cmd = LAUNCHER_OK;
	  request_header->arg_length = 0;
		printf("read default len=%d return len=%d\n",len,sizeof(*request_header));
		return sizeof(*request_header);
	}
}
#endif

// Dispose slaves after being idle for SLAVE_MAX_IDLE seconds
#define SLAVE_MAX_IDLE	30

using namespace KIO;
using namespace KNetwork;

IdleSlave::IdleSlave(KStreamSocket *socket)
{
   mConn.init(socket);
   mConn.connect(this, SLOT(gotInput()));
   mConn.send( CMD_SLAVE_STATUS );
   mPid = 0;
   mBirthDate = time(0);
   mOnHold = false;
}

void
IdleSlave::gotInput()
{
   int cmd;
   QByteArray data;
   if (mConn.read( &cmd, data) == -1)
   {
      // Communication problem with slave.
      kError(7016) << "SlavePool: No communication with slave." << endl;
      delete this;
   }
   else if (cmd == MSG_SLAVE_ACK)
   {
      delete this;
   }
   else if (cmd != MSG_SLAVE_STATUS)
   {
      kError(7016) << "SlavePool: Unexpected data from slave." << endl;
      delete this;
   }
   else
   {
      QDataStream stream( data );
      pid_t pid;
      QString protocol;
      QString host;
      qint8 b;
      stream >> pid >> protocol >> host >> b;
// Overload with (bool) onHold, (KUrl) url.
      if (!stream.atEnd())
      {
         KUrl url;
         stream >> url;
         mOnHold = true;
         mUrl = url;
      }

      mPid = pid;
      mConnected = (b != 0);
      mProtocol = protocol;
      mHost = host;
      emit statusUpdate(this);
   }
}

void
IdleSlave::connect(const QString &app_socket)
{
   QByteArray data;
   QDataStream stream( &data, QIODevice::WriteOnly);
   stream << app_socket;
   mConn.send( CMD_SLAVE_CONNECT, data );
   // Timeout!
}

void
IdleSlave::reparseConfiguration()
{
   mConn.send( CMD_REPARSECONFIGURATION );
}

bool
IdleSlave::match(const QString &protocol, const QString &host, bool connected)
{
   if (mOnHold) return false;
   if (protocol != mProtocol) return false;
   if (host.isEmpty()) return true;
   if (host != mHost) return false;
   if (!connected) return true;
   if (!mConnected) return false;
   return true;
}

bool
IdleSlave::onHold(const KUrl &url)
{
   if (!mOnHold) return false;
   return (url == mUrl);
}

int
IdleSlave::age(time_t now)
{
   return (int) difftime(now, mBirthDate);
}

KLauncher::KLauncher(int _kdeinitSocket)
  : KApplication( false ), // No GUI
    kdeinitSocket(_kdeinitSocket), dontBlockReading(false)
{
#ifdef Q_WS_X11
   mCached_dpy = NULL;
#endif
   mAutoTimer.setSingleShot(true);
   new KLauncherAdaptor(this);
   QDBusConnection::sessionBus().registerObject("/KLauncher", this); // same as ktoolinvocation.cpp

   connect(&mAutoTimer, SIGNAL(timeout()), this, SLOT(slotAutoStart()));
   connect(QDBusConnection::sessionBus().interface(),
           SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           SLOT(slotNameOwnerChanged(QString,QString,QString)));

#ifndef Q_WS_WIN
   QString prefix = KStandardDirs::locateLocal("socket", "klauncher");
   KTemporaryFile *domainname = new KTemporaryFile();
   domainname->setPrefix(prefix);
   domainname->setSuffix(QLatin1String(".slave-socket"));
   domainname->setAutoRemove(false);
   if (!domainname->open())
   {
      // Sever error!
      qDebug("KLauncher: Fatal error, can't create tempfile!");
      ::_exit(1);
   }
   mPoolSocketName = domainname->fileName();
#ifdef __CYGWIN__
   domainname->setAutoRemove(true);
#endif
   delete domainname;
   mPoolSocket.setAddress(QFile::encodeName(mPoolSocketName));
#else
   mPoolSocket.setAddress(QFile::encodeName("4545"));
#endif
   mPoolSocket.setAcceptBuffered(false); // we use KStreamSockets
   connect(&mPoolSocket, SIGNAL(readyAccept()),
           SLOT(acceptSlave()));
   mPoolSocket.listen();

   connect(&mTimer, SIGNAL(timeout()), SLOT(idleTimeout()));

#ifndef Q_WS_WIN
   kdeinitNotifier = new QSocketNotifier(kdeinitSocket, QSocketNotifier::Read);
   connect(kdeinitNotifier, SIGNAL( activated( int )),
           this, SLOT( slotKDEInitData( int )));
   kdeinitNotifier->setEnabled( true );
#endif
   lastRequest = 0;
   bProcessingQueue = false;

   mSlaveDebug = getenv("KDE_SLAVE_DEBUG_WAIT");
   if (!mSlaveDebug.isEmpty())
   {
      qWarning("Klauncher running in slave-debug mode for slaves of protocol '%s'", qPrintable(mSlaveDebug));
   }
   mSlaveValgrind = getenv("KDE_SLAVE_VALGRIND");
   if (!mSlaveValgrind.isEmpty())
   {
      mSlaveValgrindSkin = getenv("KDE_SLAVE_VALGRIND_SKIN");
      qWarning("Klauncher running slaves through valgrind for slaves of protocol '%s'", qPrintable(mSlaveValgrind));
   }
   klauncher_header request_header;
   request_header.cmd = LAUNCHER_OK;
   request_header.arg_length = 0;
   mywrite(kdeinitSocket, &request_header, sizeof(request_header));
	 qDebug("LAUNCHER_OK");
}

KLauncher::~KLauncher()
{
   close();
}

void KLauncher::close()
{
   if (!mPoolSocketName.isEmpty())
   {
      const QByteArray filename = QFile::encodeName(mPoolSocketName);
      unlink(filename.data());
   }
#ifdef Q_WS_X11
   if( mCached_dpy != NULL )
       XCloseDisplay( mCached_dpy );
#endif
}

void
KLauncher::destruct(int exit_code)
{
   if (kapp) ((KLauncher*)kapp)->close();
   // We don't delete kapp here, that's intentional.
   ::_exit(exit_code);
}

void KLauncher::setLaunchEnv(const QString &name, const QString &value)
{
   klauncher_header request_header;
   QByteArray requestData;
   requestData.append(name.toLocal8Bit()).append('\0').append(value.toLocal8Bit()).append('\0');
   request_header.cmd = LAUNCHER_SETENV;
   request_header.arg_length = requestData.size();
   mywrite(kdeinitSocket, &request_header, sizeof(request_header));
   mywrite(kdeinitSocket, requestData.data(), request_header.arg_length);
}

/*
 * Read 'len' bytes from 'sock' into buffer.
 * returns -1 on failure, 0 on no data.
 */
static int
read_socket(int sock, char *buffer, int len)
{
  ssize_t result;
  int bytes_left = len;
  while ( bytes_left > 0)
  {
     result = myread(sock, buffer, bytes_left);
     if (result > 0)
     {
        buffer += result;
        bytes_left -= result;
     }
     else if (result == 0)
        return -1;
     else if ((result == -1) && (errno != EINTR))
        return -1;
  }
  return 0;
}


void
KLauncher::slotKDEInitData(int)
{
   klauncher_header request_header;
   QByteArray requestData;
/*
   if( dontBlockReading )
   {
   // in case we get a request to start an application and data arrive
   // to kdeinitSocket at the same time, requestStart() will already
   // call slotKDEInitData(), so we must check there's still something
   // to read, otherwise this would block
      fd_set in;
      timeval tm = { 0, 0 };
      FD_ZERO ( &in );
      FD_SET( kdeinitSocket, &in );
      select( kdeinitSocket + 1, &in, 0, 0, &tm );
      if( !FD_ISSET( kdeinitSocket, &in ))
         return;
   }
   dontBlockReading = false;
*/
   int result = read_socket(kdeinitSocket, (char *) &request_header,
                            sizeof( request_header));
   if (result == -1)
   {
      kDebug() << "Exiting on read_socket errno: " << errno << endl;
      ::signal( SIGHUP, SIG_IGN);
      ::signal( SIGTERM, SIG_IGN);
      destruct(255); // Exit!
   }
   requestData.resize(request_header.arg_length);
   result = read_socket(kdeinitSocket, (char *) requestData.data(),
                        request_header.arg_length);

   if (request_header.cmd == LAUNCHER_DIED)
   {
     long *request_data;
     request_data = (long *) requestData.data();
     processDied(request_data[0], request_data[1]);
     return;
   }
   if (lastRequest && (request_header.cmd == LAUNCHER_OK))
   {
     long *request_data;
     request_data = (long *) requestData.data();
     lastRequest->pid = (pid_t) (*request_data);
     kDebug(7016) << lastRequest->name << " (pid " << lastRequest->pid <<
        ") up and running." << endl;
     switch(lastRequest->dbus_startup_type)
     {
       case KService::DBUS_None:
       {
         lastRequest->status = KLaunchRequest::Running;
         break;
       }

       case KService::DBUS_Unique:
       {
         lastRequest->status = KLaunchRequest::Launching;
         break;
       }

       case KService::DBUS_Wait:
       {
         lastRequest->status = KLaunchRequest::Launching;
         break;
       }

       case KService::DBUS_Multi:
       {
         lastRequest->status = KLaunchRequest::Launching;
         break;
       }
     }
     lastRequest = 0;
     return;
   }
   if (lastRequest && (request_header.cmd == LAUNCHER_ERROR))
   {
     lastRequest->status = KLaunchRequest::Error;
     if (!requestData.isEmpty())
        lastRequest->errorMsg = QString::fromUtf8((char *) requestData.data());
     lastRequest = 0;
     return;
   }

   kWarning(7016) << "Unexpected command from KDEInit (" << (unsigned int) request_header.cmd
                 << ")" << endl;
}

void
KLauncher::processDied(pid_t pid, long /* exitStatus */)
{
   foreach (KLaunchRequest *request, requestList)
   {
      if (request->pid == pid)
      {
         if (request->dbus_startup_type == KService::DBUS_Wait)
            request->status = KLaunchRequest::Done;
         else if ((request->dbus_startup_type == KService::DBUS_Unique) && QDBusConnection::sessionBus().interface()->isServiceRegistered(request->dbus_name))
            request->status = KLaunchRequest::Running;
         else
            request->status = KLaunchRequest::Error;
         requestDone(request);
         return;
      }
   }
}

void
KLauncher::slotNameOwnerChanged(const QString &appId, const QString &oldOwner,
                                const QString &newOwner)
{
   Q_UNUSED(oldOwner);
   if (appId.isEmpty() || newOwner.isEmpty())
      return;

   foreach (KLaunchRequest *request, requestList)
   {
      if (request->status != KLaunchRequest::Launching)
         continue;

      // For unique services check the requested service name first
      if ((request->dbus_startup_type == KService::DBUS_Unique) &&
          ((appId == request->dbus_name) ||
           QDBusConnection::sessionBus().interface()->isServiceRegistered(request->dbus_name)))
      {
         request->status = KLaunchRequest::Running;
         requestDone(request);
         continue;
      }

      const QString rAppId = request->dbus_name;
      if (rAppId.isEmpty())
          return;

      //int l = strlen(rAppId);

      QChar c = appId.length() > rAppId.length() ? appId.at(rAppId.length()) : QChar();
      if (appId.startsWith(rAppId) && ((appId.length() == rAppId.length()) ||
                  (c == QLatin1Char('-'))))
      {
         request->dbus_name = appId;
         request->status = KLaunchRequest::Running;
         requestDone(request);
         continue;
      }
   }
}

void
KLauncher::autoStart(int phase)
{
   if( mAutoStart.phase() >= phase )
       return;
   mAutoStart.setPhase(phase);
   if (phase == 0)
      mAutoStart.loadAutoStartList();
   mAutoTimer.start(0);
}

void
KLauncher::slotAutoStart()
{
   KService::Ptr s;
   do
   {
      QString service = mAutoStart.startService();
      if (service.isEmpty())
      {
         // Done
	 if( !mAutoStart.phaseDone())
	 {
	    mAutoStart.setPhaseDone();
            switch( mAutoStart.phase())
                {
                case 0:
                    emit autoStart0Done();
                    break;
                case 1:
                    emit autoStart1Done();
                    break;
                case 2:
                    emit autoStart2Done();
                    break;
                }
	 }
         return;
      }
      s = new KService(service);
   }
   while (!start_service(s, QStringList(), QStringList(), "0", false, true, QDBusMessage()));
   // Loop till we find a service that we can start.
}

void
KLauncher::requestDone(KLaunchRequest *request)
{
   if ((request->status == KLaunchRequest::Running) ||
       (request->status == KLaunchRequest::Done))
   {
      requestResult.result = 0;
      requestResult.dbusName = request->dbus_name;
      requestResult.error = "";
      requestResult.pid = request->pid;
   }
   else
   {
      requestResult.result = 1;
      requestResult.dbusName = "";
      requestResult.error = i18n("KDEInit could not launch '%1'.",
	  request->name);
      if (!request->errorMsg.isEmpty())
         requestResult.error += ":\n" + request->errorMsg;
      requestResult.pid = 0;

#ifdef Q_WS_X11
      if (!request->startup_dpy.isEmpty())
      {
         Display* dpy = NULL;
         if( (mCached_dpy != NULL) &&
              (request->startup_dpy == XDisplayString( mCached_dpy )))
            dpy = mCached_dpy;
         if( dpy == NULL )
            dpy = XOpenDisplay( request->startup_dpy.toLocal8Bit() );
         if( dpy )
         {
            KStartupInfoId id;
            id.initId( request->startup_id.toLocal8Bit() );
            KStartupInfo::sendFinishX( dpy, id );
            if( mCached_dpy != dpy && mCached_dpy != NULL )
               XCloseDisplay( mCached_dpy );
            mCached_dpy = dpy;
         }
      }
#endif
   }

   if (request->autoStart)
   {
      mAutoTimer.start(0);
   }

   if (request->transaction.type() != QDBusMessage::InvalidMessage)
   {
			printf("%s return dbus message\n",__FUNCTION__);
      if ( requestResult.dbusName.isNull() ) // null strings can't be sent
          requestResult.dbusName = "";
      Q_ASSERT( !requestResult.error.isNull() );
      QDBusConnection::sessionBus().send(request->transaction.createReply(QVariantList() << requestResult.result
                                     << requestResult.dbusName
                                     << requestResult.error
                                     << requestResult.pid));
   }
   requestList.removeAll( request );
   delete request;
}

static void appendLong(QByteArray &ba, long l)
{
   const int sz = ba.size();
   ba.resize(sz + sizeof(long));
   memcpy(ba.data() + sz, &l, sizeof(long));
}

void
KLauncher::requestStart(KLaunchRequest *request)
{
   requestList.append( request );
   // Send request to kdeinit.
   klauncher_header request_header;
   QByteArray requestData;
   requestData.reserve(1024);

   appendLong(requestData, request->arg_list.count() + 1);
   requestData.append(request->name.toLocal8Bit());
   requestData.append('\0');
   foreach (QString arg, request->arg_list)
       requestData.append(arg.toLocal8Bit()).append('\0');
   appendLong(requestData, request->envs.count());
   foreach (QString env, request->envs)
       requestData.append(env.toLocal8Bit()).append('\0');
   appendLong(requestData, 0); // avoid_loops, always false here
#ifdef Q_WS_X11
   bool startup_notify = !request->startup_id.isNull() && request->startup_id != "0";
   if( startup_notify )
       requestData.append(request->startup_id.toLocal8Bit()).append('\0');
#endif
   if (!request->cwd.isEmpty())
       requestData.append(request->cwd.toLocal8Bit()).append('\0');

#ifdef Q_WS_X11
   request_header.cmd = startup_notify ? LAUNCHER_EXT_EXEC : LAUNCHER_EXEC_NEW;
#else
   request_header.cmd = LAUNCHER_EXEC_NEW;
#endif
   request_header.arg_length = requestData.length();
   mywrite(kdeinitSocket, &request_header, sizeof(request_header));
   mywrite(kdeinitSocket, requestData.data(), requestData.length());

   // Wait for pid to return.
   lastRequest = request;
   dontBlockReading = false;
   do {
      slotKDEInitData( kdeinitSocket );
   }
   while (lastRequest != 0);
   dontBlockReading = true;
}

void KLauncher::exec_blind(const QString &name, const QStringList &arg_list, const QStringList &envs, const QString &startup_id)
{
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = false;
   request->name = name;
   request->arg_list =  arg_list;
   request->dbus_startup_type = KService::DBUS_None;
   request->pid = 0;
   request->status = KLaunchRequest::Launching;
   request->envs = envs;
   // Find service, if any - strip path if needed
   KService::Ptr service = KService::serviceByDesktopName( name.mid( name.lastIndexOf( '/' ) + 1 ));
   if (service)
       send_service_startup_info( request, service, startup_id, QStringList());
   else // no .desktop file, no startup info
       cancel_service_startup_info( request, startup_id, envs );

   requestStart(request);
   // We don't care about this request any longer....
   requestDone(request);
}


bool
KLauncher::start_service_by_name(const QString &serviceName, const QStringList &urls,
    const QStringList &envs, const QString& startup_id, bool blind, const QDBusMessage &msg)
{
   KService::Ptr service;
   // Find service
   service = KService::serviceByName(serviceName);
   if (!service)
   {
      requestResult.result = ENOENT;
      requestResult.error = i18n("Could not find service '%1'.", serviceName);
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind, false, msg);
}

bool
KLauncher::start_service_by_desktop_path(const QString &serviceName, const QStringList &urls,
    const QStringList &envs, const QString& startup_id, bool blind, const QDBusMessage &msg)
{
   KService::Ptr service;
   // Find service
   if (serviceName.startsWith(QLatin1Char('/')))
   {
      // Full path
      service = new KService(serviceName);
   }
   else
   {
      service = KService::serviceByDesktopPath(serviceName);
   }
   if (!service)
   {
      requestResult.result = ENOENT;
      requestResult.error = i18n("Could not find service '%1'.", serviceName);
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind, false, msg);
}

bool
KLauncher::start_service_by_desktop_name(const QString &serviceName, const QStringList &urls,
    const QStringList &envs, const QString& startup_id, bool blind, const QDBusMessage &msg)
{
   KService::Ptr service = KService::serviceByDesktopName(serviceName);
   if (!service)
   {
      requestResult.result = ENOENT;
      requestResult.error = i18n("Could not find service '%1'.", serviceName);
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind, false, msg);
}

bool
KLauncher::start_service(KService::Ptr service, const QStringList &_urls,
    const QStringList &envs, const QString &startup_id,
    bool blind, bool autoStart, const QDBusMessage &msg)
{
   QStringList urls = _urls;
   if (!service->isValid())
   {
      requestResult.result = ENOEXEC;
      requestResult.error = i18n("Service '%1' is malformatted.", service->desktopEntryPath());
      cancel_service_startup_info( NULL, startup_id, envs ); // cancel it if any
      return false;
   }
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = autoStart;

   if ((urls.count() > 1) && !service->allowMultipleFiles())
   {
      // We need to launch the application N times. That sucks.
      // We ignore the result for application 2 to N.
      // For the first file we launch the application in the
      // usual way. The reported result is based on this
      // application.
      QStringList::ConstIterator it = urls.begin();
      for(++it;
          it != urls.end();
          ++it)
      {
         QStringList singleUrl;
         singleUrl.append(*it);
         QString startup_id2 = startup_id;
         if( !startup_id2.isEmpty() && startup_id2 != "0" )
             startup_id2 = "0"; // can't use the same startup_id several times
         start_service( service, singleUrl, envs, startup_id2, true, false, msg);
      }
      QString firstURL = *(urls.begin());
      urls.clear();
      urls.append(firstURL);
   }
   createArgs(request, service, urls);

   // We must have one argument at least!
   if (!request->arg_list.count())
   {
      requestResult.result = ENOEXEC;
      requestResult.error = i18n("Service '%1' is malformatted.", service->desktopEntryPath());
      delete request;
      cancel_service_startup_info( NULL, startup_id, envs );
      return false;
   }

   request->name = request->arg_list.first();
   request->arg_list.removeFirst(); //(request->arg_list.begin());

   request->dbus_startup_type =  service->DBUSStartupType();

   if ((request->dbus_startup_type == KService::DBUS_Unique) ||
       (request->dbus_startup_type == KService::DBUS_Multi))
   {
      QVariant v = service->property("X-DCOP-ServiceName");
      if (v.isValid())
         request->dbus_name = v.toString().toUtf8();
      if (request->dbus_name.isEmpty())
      {
         request->dbus_name = QFile::encodeName(KRun::binaryName(service->exec(), true));
      }
   }

   request->pid = 0;
   request->envs = envs;
   send_service_startup_info( request, service, startup_id, envs );

   // Request will be handled later.
   if (!blind && !autoStart)
   {
      msg.setDelayedReply(true);
      request->transaction = msg;
   }
   queueRequest(request);
   return true;
}

void
KLauncher::send_service_startup_info( KLaunchRequest *request, KService::Ptr service, const QString& startup_id,
    const QStringList &envs )
{
    return;
}

void
KLauncher::cancel_service_startup_info( KLaunchRequest* request, const QString& startup_id,
    const QStringList &envs )
{
}

bool
KLauncher::kdeinit_exec(const QString &app, const QStringList &args,
   const QStringList &envs, const QString &startup_id, bool wait, const QDBusMessage &msg)
{
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = false;

   for(QStringList::ConstIterator it = args.begin();
       it != args.end();
       it++)
   {
       QString arg = *it;
       request->arg_list.append(arg.toLocal8Bit());
   }

   request->name = app.toLocal8Bit();
   if (wait)
      request->dbus_startup_type = KService::DBUS_Wait;
   else
      request->dbus_startup_type = KService::DBUS_None;
   request->pid = 0;

   request->envs = envs;
   if( app != "kbuildsycoca" ) // avoid stupid loop
   {
       // Find service, if any - strip path if needed
       KService::Ptr service = KService::serviceByDesktopName( app.mid( app.lastIndexOf( '/' ) + 1 ));
       if (service)
           send_service_startup_info( request,  service,
               startup_id, QStringList());
       else // no .desktop file, no startup info
           cancel_service_startup_info( request, startup_id, envs );
   }
   msg.setDelayedReply(true);
   request->transaction = msg;
   queueRequest(request);
   return true;
}

void
KLauncher::queueRequest(KLaunchRequest *request)
{
   requestQueue.append( request );
   if (!bProcessingQueue)
   {
      bProcessingQueue = true;
      QTimer::singleShot(0, this, SLOT( slotDequeue() ));
   }
}

void
KLauncher::slotDequeue()
{
   do {
      KLaunchRequest *request = requestQueue.takeFirst();
      // process request
      request->status = KLaunchRequest::Launching;
      requestStart(request);
      if (request->status != KLaunchRequest::Launching)
      {
         // Request handled.
         requestDone( request );
         continue;
      }
   } while(requestQueue.count());
   bProcessingQueue = false;
}

void
KLauncher::createArgs( KLaunchRequest *request, const KService::Ptr service ,
                       const QStringList &urls)
{
  const QStringList params = KRun::processDesktopExec(*service, urls);

  for(QStringList::ConstIterator it = params.begin();
      it != params.end(); ++it)
  {
     request->arg_list.append((*it).toLocal8Bit());
  }
  request->cwd = QFile::encodeName(service->path());
}

///// IO-Slave functions

pid_t
KLauncher::requestHoldSlave(const KUrl &url, const QString &app_socket)
{
    IdleSlave *slave = 0;
    foreach (slave, mSlaveList)
    {
       if (slave->onHold(url))
          break;
    }
    if (slave)
    {
       mSlaveList.removeAll(slave);
       slave->connect(app_socket);
       return slave->pid();
    }
    return 0;
}


pid_t
KLauncher::requestSlave(const QString &protocol,
                        const QString &host,
                        const QString &app_socket,
                        QString &error)
{
    IdleSlave *slave = 0;
    foreach (slave, mSlaveList)
    {
       if (slave->match(protocol, host, true))
          break;
    }
    if (!slave)
    {
       foreach (slave, mSlaveList)
       {
          if (slave->match(protocol, host, false))
             break;
       }
    }
    if (!slave)
    {
       foreach (slave, mSlaveList)
       {
          if (slave->match(protocol, QString(), false))
             break;
       }
    }
    if (slave)
    {
       mSlaveList.removeAll(slave);
       slave->connect(app_socket);
       return slave->pid();
    }

    QString name = KProtocolInfo::exec(protocol);
    if (name.isEmpty())
    {
	error = i18n("Unknown protocol '%1'.\n", protocol);
        return 0;
    }

    QString arg1 = protocol;
    QString arg2 = QString::fromLocal8Bit(QFile::encodeName(mPoolSocketName));
    QString arg3 = QString::fromLocal8Bit(QFile::encodeName(app_socket));
    QStringList arg_list;
    arg_list.append(arg1);
    arg_list.append(arg2);
    arg_list.append(arg3);

    kDebug(7016) << "KLauncher: launching new slave " << name << " with protocol=" << protocol
        << " args=" << arg_list << endl;
    if (mSlaveDebug == arg1)
    {
       klauncher_header request_header;
       request_header.cmd = LAUNCHER_DEBUG_WAIT;
       request_header.arg_length = 0;
       mywrite(kdeinitSocket, &request_header, sizeof(request_header));
    }
    if (mSlaveValgrind == arg1)
    {
       arg_list.prepend(QFile::encodeName(KLibLoader::findLibrary(name.toLocal8Bit())));
       arg_list.prepend(QFile::encodeName(KStandardDirs::locate("exe", "kioslave")));
       name = "valgrind";
       if (!mSlaveValgrindSkin.isEmpty()) {
           arg_list.prepend(QLatin1String("--tool=") + mSlaveValgrindSkin);
       } else
	   arg_list.prepend(QLatin1String("--tool=memcheck"));
    }

    KLaunchRequest *request = new KLaunchRequest;
    request->autoStart = false;
    request->name = name;
    request->arg_list =  arg_list;
    request->dbus_startup_type = KService::DBUS_None;
    request->pid = 0;
#ifdef Q_WS_X11
    request->startup_id = "0";
#endif
    request->status = KLaunchRequest::Launching;
    requestStart(request);
    pid_t pid = request->pid;

//    kDebug(7016) << "Slave launched, pid = " << pid << endl;

    // We don't care about this request any longer....
    requestDone(request);
    if (!pid)
    {
       error = i18n("Error loading '%1'.\n", name);
    }
    return pid;
}

void
KLauncher::waitForSlave(int pid, const QDBusMessage &msg)
{
    foreach (IdleSlave *slave, mSlaveList)
    {
        if (slave->pid() == static_cast<pid_t>(pid))
           return; // Already here.
    }
    SlaveWaitRequest *waitRequest = new SlaveWaitRequest;
    msg.setDelayedReply(true);
    waitRequest->transaction = msg;
    waitRequest->pid = static_cast<pid_t>(pid);
    mSlaveWaitRequest.append(waitRequest);
}

void
KLauncher::acceptSlave()
{
    // the cast is safe, because KServerSocket promises us so
    KStreamSocket* slaveSocket = static_cast<KStreamSocket*>(mPoolSocket.accept());
    IdleSlave *slave = new IdleSlave(slaveSocket);
    // Send it a SLAVE_STATUS command.
    mSlaveList.append(slave);
    connect(slave, SIGNAL(destroyed()), this, SLOT(slotSlaveGone()));
    connect(slave, SIGNAL(statusUpdate(IdleSlave *)),
	    this, SLOT(slotSlaveStatus(IdleSlave *)));
    if (!mTimer.isActive())
    {
       mTimer.start(1000*10);
    }
}

void
KLauncher::slotSlaveStatus(IdleSlave *slave)
{
    QMutableListIterator<SlaveWaitRequest *> it(mSlaveWaitRequest);
    while(it.hasNext())
    {
       SlaveWaitRequest *waitRequest = it.next();
       if (waitRequest->pid == slave->pid())
       {
           QDBusConnection::sessionBus().send(waitRequest->transaction.createReply());
          it.remove();
          delete waitRequest;
       }
    }
}

void
KLauncher::slotSlaveGone()
{
    IdleSlave *slave = (IdleSlave *) sender();
    mSlaveList.removeAll(slave);
    if ((mSlaveList.count() == 0) && (mTimer.isActive()))
    {
       mTimer.stop();
    }
}

void
KLauncher::idleTimeout()
{
    bool keepOneFileSlave=true;
    time_t now = time(0);
    foreach (IdleSlave *slave, mSlaveList)
    {
        if ((slave->protocol()=="file") && (keepOneFileSlave))
           keepOneFileSlave=false;
        else if (slave->age(now) > SLAVE_MAX_IDLE)
        {
           // killing idle slave
           delete slave;
        }
    }
}

void KLauncher::reparseConfiguration()
{
   KProtocolManager::reparseConfiguration();
   foreach (IdleSlave *slave, mSlaveList)
      slave->reparseConfiguration();
}

#include "klauncher.moc"
