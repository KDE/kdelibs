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
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <qfile.h>

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kprotocolmanager.h>
#include <kprotocolinfo.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kstartupinfo.h>

#include "kio/global.h"
#include "kio/connection.h"
#include "kio/slaveinterface.h"

#include "klauncher.h"
#include "klauncher_cmds.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

// Dispose slaves after being idle for SLAVE_MAX_IDLE seconds
#define SLAVE_MAX_IDLE	30

using namespace KIO;

template class QPtrList<KLaunchRequest>;
template class QPtrList<IdleSlave>;

IdleSlave::IdleSlave(KSocket *socket)
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
      kdError(7016) << "SlavePool: No communication with slave." << endl;
      delete this;
   }
   else if (cmd == MSG_SLAVE_ACK)
   {
      delete this;
   }
   else if (cmd != MSG_SLAVE_STATUS)
   {
      kdError(7016) << "SlavePool: Unexpected data from slave." << endl;
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
// Overload with (bool) onHold, (KURL) url.
      if (!stream.atEnd())
      {
         KURL url;
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
   QDataStream stream( data, IO_WriteOnly);
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
IdleSlave::onHold(const KURL &url)
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
  : KUniqueApplication( false, false ), // No Styles, No GUI
    kdeinitSocket(_kdeinitSocket)
{
   requestList.setAutoDelete(true);
   mSlaveWaitRequest.setAutoDelete(true);
   dcopClient()->setNotifications( true );
   connect(dcopClient(), SIGNAL( applicationRegistered( const QCString &)),
           this, SLOT( slotAppRegistered( const QCString &)));
   dcopClient()->connectDCOPSignal( "DCOPServer", "", "terminateKDE()",
                                    objId(), "terminateKDE()", false );

   QString prefix = locateLocal("socket", "klauncher");
   KTempFile domainname(prefix, QString::fromLatin1(".slave-socket"));
   if (domainname.status() != 0)
   {
      // Sever error!
      qDebug("KLauncher: Fatal error, can't create tempfile!");
      ::exit(1);
   }
   mPoolSocketName = domainname.name();
   mPoolSocket = new KServerSocket(QFile::encodeName(mPoolSocketName));
   connect(mPoolSocket, SIGNAL(accepted( KSocket *)),
           SLOT(acceptSlave(KSocket *)));

   connect(&mTimer, SIGNAL(timeout()), SLOT(idleTimeout()));

   kdeinitNotifier = new QSocketNotifier(kdeinitSocket, QSocketNotifier::Read);
   connect(kdeinitNotifier, SIGNAL( activated( int )),
           this, SLOT( slotKDEInitData( int )));
   kdeinitNotifier->setEnabled( true );
   lastRequest = 0;
   bProcessingQueue = false;

   mSlaveDebug = getenv("KDE_SLAVE_DEBUG_WAIT");
   if (!mSlaveDebug.isEmpty())
   {
      qWarning("Klauncher running in slave-debug mode for slaves of protocol '%s'", mSlaveDebug.data());
   }
}

KLauncher::~KLauncher()
{
   if (!mPoolSocketName.isEmpty())
   {
      QCString filename = QFile::encodeName(mPoolSocketName);
      unlink(filename.data());
   }
}

void
KLauncher::destruct(int exit_code)
{
   ::signal( SIGHUP, SIG_IGN);
   ::signal( SIGPIPE, SIG_IGN);
   ::signal( SIGTERM, SIG_IGN);
   if (kapp)
   {
      // work around braindead QApplication mutex handling
#if defined(QT_THREAD_SUPPORT)
      if (kapp->locked())
         kapp->unlock(false);
#endif
      delete kapp;
   }
   ::exit(exit_code);
}

bool
KLauncher::process(const QCString &fun, const QByteArray &data,
                   QCString &replyType, QByteArray &replyData)
{
   if ((fun == "exec_blind(QCString,QValueList<QCString>)")
       || (fun == "exec_blind(QCString,QValueList<QCString>,QValueList<QCString>,QCString)"))
   {
      QDataStream stream(data, IO_ReadOnly);

      QCString name;
      QValueList<QCString> arg_list;
      QCString startup_id = "";
      QValueList<QCString> envs;
      stream >> name >> arg_list;
      if( fun == "exec_blind(QCString,QValueList<QCString>,QValueList<QCString>,QCString)" )
          stream >> envs >> startup_id;
      kdDebug(7016) << "KLauncher: Got exec_blind('" << name << "', ...)" << endl;
      exec_blind( name, arg_list, envs, startup_id);
      return true;
   }
   if ((fun == "start_service_by_name(QString,QStringList)") ||
       (fun == "start_service_by_desktop_path(QString,QStringList)")||
       (fun == "start_service_by_desktop_name(QString,QStringList)")||
       (fun == "kdeinit_exec(QString,QStringList)") ||
       (fun == "kdeinit_exec_wait(QString,QStringList)") ||
       (fun == "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString)") ||
       (fun == "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString)")||
       (fun == "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString)") ||
       (fun == "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString,bool)") ||
       (fun == "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString,bool)")||
       (fun == "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString,bool)") ||
       (fun == "kdeinit_exec(QString,QStringList,QValueList<QCString>)") ||
       (fun == "kdeinit_exec_wait(QString,QStringList,QValueList<QCString>)"))
   {
      QDataStream stream(data, IO_ReadOnly);
      bool bNoWait = false;
      QString serviceName;
      QStringList urls;
      QValueList<QCString> envs;
      QCString startup_id = "";
      DCOPresult.result = -1;
      DCOPresult.dcopName = 0;
      DCOPresult.error = QString::null;
      DCOPresult.pid = 0;
      stream >> serviceName >> urls;
      if ((fun == "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString,bool)") ||
          (fun == "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString,bool)")||
          (fun == "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString,bool)"))
         stream >> envs >> startup_id >> bNoWait;
      else if ((fun == "start_service_by_name(QString,QStringList,QValueList<QCString>,QCString)") ||
          (fun == "start_service_by_desktop_path(QString,QStringList,QValueList<QCString>,QCString)")||
          (fun == "start_service_by_desktop_name(QString,QStringList,QValueList<QCString>,QCString)"))
         stream >> envs >> startup_id;
      else if ((fun == "kdeinit_exec(QString,QStringList,QValueList<QCString>)") ||
          (fun == "kdeinit_exec_wait(QString,QStringList,QValueList<QCString>)"))
         stream >> envs;
      bool finished;
      if (strncmp(fun, "start_service_by_name(", 22) == 0)
      {
         kdDebug(7016) << "KLauncher: Got start_service_by_name('" << serviceName << "', ...)" << endl;
         finished = start_service_by_name(serviceName, urls, envs, startup_id, bNoWait);
      }
      else if (strncmp(fun, "start_service_by_desktop_path(", 30) == 0)
      {
         kdDebug(7016) << "KLauncher: Got start_service_by_desktop_path('" << serviceName << "', ...)" << endl;
         finished = start_service_by_desktop_path(serviceName, urls, envs, startup_id, bNoWait);
      }
      else if (strncmp(fun, "start_service_by_desktop_name(", 30) == 0)
      {
         kdDebug(7016) << "KLauncher: Got start_service_by_desktop_name('" << serviceName << "', ...)" << endl;
         finished = start_service_by_desktop_name(serviceName, urls, envs, startup_id, bNoWait );
      }
      else if ((fun == "kdeinit_exec(QString,QStringList)")
              || (fun == "kdeinit_exec(QString,QStringList,QValueList<QCString>)"))
      {
         kdDebug(7016) << "KLauncher: Got kdeinit_exec('" << serviceName << "', ...)" << endl;
         finished = kdeinit_exec(serviceName, urls, envs, false);
      }
      else
      {
         kdDebug(7016) << "KLauncher: Got kdeinit_exec_wait('" << serviceName << "', ...)" << endl;
         finished = kdeinit_exec(serviceName, urls, envs, true);
      }
      if (!finished)
      {
         replyType = "serviceResult";
         QDataStream stream2(replyData, IO_WriteOnly);
         stream2 << DCOPresult.result << DCOPresult.dcopName << DCOPresult.error << DCOPresult.pid;
      }
      return true;
   }
   else if (fun == "requestSlave(QString,QString,QString)")
   {
      QDataStream stream(data, IO_ReadOnly);
      QString protocol;
      QString host;
      QString app_socket;
      stream >> protocol >> host >> app_socket;
      replyType = "QString";
      QString error;
      pid_t pid = requestSlave(protocol, host, app_socket, error);
      QDataStream stream2(replyData, IO_WriteOnly);
      stream2 << pid << error;
      return true;
   }
   else if (fun == "requestHoldSlave(KURL,QString)")
   {
      QDataStream stream(data, IO_ReadOnly);
      KURL url;
      QString app_socket;
      stream >> url >> app_socket;
      replyType = "pid";
      pid_t pid = requestHoldSlave(url, app_socket);
      QDataStream stream2(replyData, IO_WriteOnly);
      stream2 << pid;
      return true;
   }
   else if (fun == "waitForSlave(pid_t)")
   {
      QDataStream stream(data, IO_ReadOnly);
      pid_t pid;
      stream >> pid;
      waitForSlave(pid);
      replyType = "void";
      return true;

   }
   else if (fun == "setLaunchEnv(QCString,QCString)")
   {
      QDataStream stream(data, IO_ReadOnly);
      QCString name;
      QCString value;
      stream >> name >> value;
      setLaunchEnv(name, value);
      replyType = "void";
      return true;
   }
   else if (fun == "reparseConfiguration()")
   {
      kdDebug(7016) << "KLauncher::process : reparseConfiguration" << endl;
      KProtocolManager::reparseConfiguration();
      IdleSlave *slave;
      for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
          slave->reparseConfiguration();
      replyType = "void";
      return true;
   }
   else if (fun == "terminateKDE()")
   {
      kdDebug() << "KLauncher::process ---> terminateKDE" << endl;
      klauncher_header request_header;
      request_header.cmd = LAUNCHER_TERMINATE_KDE;
      request_header.arg_length = 0;
      write(kdeinitSocket, &request_header, sizeof(request_header));
      destruct(0);
   }
   else if (fun == "autoStart()")
   {
      kdDebug() << "KLauncher::process ---> autoStart" << endl;
      autoStart();
      replyType = "void";
      return true;
   }

   if (KUniqueApplication::process(fun, data, replyType, replyData))
   {
      return true;
   }
   kdWarning(7016) << "Got unknown DCOP function: " << fun << endl;
   return false;
}

void KLauncher::setLaunchEnv(const QCString &name, const QCString &_value)
{
   QCString value(_value);
   if (value.isNull())
      value = "";
   klauncher_header request_header;
   QByteArray requestData(name.length()+value.length()+2);
   memcpy(requestData.data(), name.data(), name.length()+1);
   memcpy(requestData.data()+name.length()+1, value.data(), value.length()+1);
   request_header.cmd = LAUNCHER_SETENV;
   request_header.arg_length = requestData.size();
   write(kdeinitSocket, &request_header, sizeof(request_header));
   write(kdeinitSocket, requestData.data(), request_header.arg_length);
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
     result = read(sock, buffer, bytes_left);
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
   int result = read_socket(kdeinitSocket, (char *) &request_header,
                            sizeof( request_header));
   if (result == -1)
   {
      kdError(7016) << "KLauncher: KDEInit communication error! Commiting suicide!" << endl;
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
     kdDebug(7016) << lastRequest->name << " (pid " << lastRequest->pid <<
        ") up and running." << endl;
     switch(lastRequest->dcop_service_type)
     {
       case KService::DCOP_None:
       {
         lastRequest->status = KLaunchRequest::Running;
         break;
       }

       case KService::DCOP_Unique:
       {
         lastRequest->status = KLaunchRequest::Launching;
         break;
       }

       case KService::DCOP_Wait:
       {
         lastRequest->status = KLaunchRequest::Launching;
         break;
       }

       case KService::DCOP_Multi:
       {
         lastRequest->status = KLaunchRequest::Launching;
         QCString pidStr;
         pidStr.setNum(lastRequest->pid);
         lastRequest->dcop_name = lastRequest->name + "-" + pidStr;
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

   kdWarning(7016) << "Unexpected command from KDEInit (" << (unsigned int) request_header.cmd
                 << ")" << endl;
}

void
KLauncher::processDied(pid_t pid, long /* exitStatus */)
{
   KLaunchRequest *request = requestList.first();
   for(; request; request = requestList.next())
   {
      if (request->pid == pid)
      {
         if (request->dcop_service_type == KService::DCOP_Wait)
            request->status = KLaunchRequest::Done;
         else if ((request->dcop_service_type == KService::DCOP_Unique) &&
		(dcopClient()->isApplicationRegistered(request->dcop_name)))
            request->status = KLaunchRequest::Running;
         else
            request->status = KLaunchRequest::Error;
         requestDone(request);
         return;
      }
   }
}

void
KLauncher::slotAppRegistered(const QCString &appId)
{
   KLaunchRequest *request = requestList.first();
   for(; request; request = requestList.next())
   {
      if ((request->dcop_name == appId) &&
          (request->status == KLaunchRequest::Launching))
      {
         request->status = KLaunchRequest::Running;
         requestDone(request);
         return;
      }
   }
}

void
KLauncher::autoStart()
{
   mAutoStart.loadAutoStartList();
   connect(&mAutoTimer, SIGNAL(timeout()), this, SLOT(slotAutoStart()));
   mAutoTimer.start(0, true);
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
         // Emit signal
         emitDCOPSignal("autoStartDone()", QByteArray());
         return;
      }
      s = new KService(service);
   }
   while (!start_service(s, QStringList(), QValueList<QCString>(), "", false, true));
   // Loop till we find a service that we can start.
}

void
KLauncher::requestDone(KLaunchRequest *request)
{
   if ((request->status == KLaunchRequest::Running) ||
       (request->status == KLaunchRequest::Done))
   {
      DCOPresult.result = 0;
      DCOPresult.dcopName = request->dcop_name;
      DCOPresult.error = QString::null;
      DCOPresult.pid = request->pid;
   }
   else
   {
      DCOPresult.result = 1;
      DCOPresult.dcopName = "";
      DCOPresult.error = i18n("KDEInit could not launch '%1'").arg(request->name);
      if (!request->errorMsg.isEmpty())
         DCOPresult.error += ":\n" + request->errorMsg;
      DCOPresult.pid = 0;
   }
   if (request->autoStart)
   {
      mAutoTimer.start(0, true);
   }
   
   if (request->transaction)
   {
      QByteArray replyData;
      QCString replyType;
      replyType = "serviceResult";
      QDataStream stream2(replyData, IO_WriteOnly);
      stream2 << DCOPresult.result << DCOPresult.dcopName << DCOPresult.error << DCOPresult.pid;
      dcopClient()->endTransaction( request->transaction,
                                    replyType, replyData);
   }
   requestList.removeRef( request );
}

void
KLauncher::requestStart(KLaunchRequest *request)
{
   requestList.append( request );
   // Send request to kdeinit.
   klauncher_header request_header;
   QByteArray requestData;
   int length = 0;
   length += sizeof(long); // Nr of. Args
   length += request->name.length() + 1; // Cmd
   for(QValueList<QCString>::Iterator it = request->arg_list.begin();
       it != request->arg_list.end();
       it++)
   {
      length += (*it).length() + 1; // Args...
   }
   length += sizeof(long); // Nr of. envs
   for(QValueList<QCString>::ConstIterator it = request->envs.begin();
       it != request->envs.end();
       it++)
   {
      length += (*it).length() + 1; // Envs...
   }
   length += sizeof( long ); // avoid_loops
   bool startup_notify = !request->startup_id.isNull() && request->startup_id != "0";
   if( startup_notify )
       length += request->startup_id.length() + 1;
       
   requestData.resize( length );

   char *p = requestData.data();
   long l = request->arg_list.count()+1;
   memcpy(p, &l, sizeof(long));
   p += sizeof(long);
   strcpy(p, request->name.data());
   p += strlen(p) + 1;
   for(QValueList<QCString>::Iterator it = request->arg_list.begin();
       it != request->arg_list.end();
       it++)
   {
      strcpy(p, (*it).data());
      p += strlen(p) + 1;
   }
   l = request->envs.count();
   memcpy(p, &l, sizeof(long));
   p += sizeof(long);
   for(QValueList<QCString>::ConstIterator it = request->envs.begin();
       it != request->envs.end();
       it++)
   {
      strcpy(p, (*it).data());
      p += strlen(p) + 1;
   }
   l = 0; // avoid_loops, always false here
   memcpy(p, &l, sizeof(long));
   p += sizeof(long);
   if( startup_notify )
   {
       strcpy(p, request->startup_id.data());
       p += strlen( p ) + 1;
   }
   request_header.cmd = startup_notify ? LAUNCHER_EXT_EXEC : LAUNCHER_EXEC_NEW;
   request_header.arg_length = length;
   write(kdeinitSocket, &request_header, sizeof(request_header));
   write(kdeinitSocket, requestData.data(), request_header.arg_length);

   // Wait for pid to return.
   lastRequest = request;
   do {
      slotKDEInitData( kdeinitSocket );
   }
   while (lastRequest != 0);
}

void
KLauncher::exec_blind( const QCString &name, const QValueList<QCString> &arg_list,
    const QValueList<QCString> &envs, const QCString& startup_id )
{
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = false;
   request->name = name;
   request->arg_list =  arg_list;
   request->dcop_name = 0;
   request->dcop_service_type = KService::DCOP_None;
   request->pid = 0;
   request->status = KLaunchRequest::Launching;
   request->transaction = 0; // No confirmation is send
   request->envs = envs;
   // Find service, if any
   KService::Ptr service = 0;
   if (name[0] == '/') // Full path
      service = new KService(name);
   else
      service = KService::serviceByDesktopName(name);
   if (service != NULL)
       request->startup_id = send_service_startup_info( service,
           startup_id, QValueList< QCString >());
   else
       request->startup_id = "0"; // no .desktop file, no startup info
   requestStart(request);
   // We don't care about this request any longer....
   requestDone(request);
}


bool
KLauncher::start_service_by_name(const QString &serviceName, const QStringList &urls,
    const QValueList<QCString> &envs, const QCString& startup_id, bool blind)
{
   KService::Ptr service = 0;
   // Find service
   service = KService::serviceByName(serviceName);
   if (!service)
   {
      DCOPresult.result = ENOENT;
      DCOPresult.error = i18n("Could not find service '%1'.").arg(serviceName);
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind);
}

bool
KLauncher::start_service_by_desktop_path(const QString &serviceName, const QStringList &urls,
    const QValueList<QCString> &envs, const QCString& startup_id, bool blind)
{
   KService::Ptr service = 0;
   // Find service
   if (serviceName[0] == '/')
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
      DCOPresult.result = ENOENT;
      DCOPresult.error = i18n("Could not find service '%1'.").arg(serviceName);
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind);
}

bool
KLauncher::start_service_by_desktop_name(const QString &serviceName, const QStringList &urls,
    const QValueList<QCString> &envs, const QCString& startup_id, bool blind)
{
   KService::Ptr service = 0;
   // Find service
   service = KService::serviceByDesktopName(serviceName);
   if (!service)
   {
      DCOPresult.result = ENOENT;
      DCOPresult.error = i18n("Could not find service '%1'.").arg(serviceName);
      return false;
   }
   return start_service(service, urls, envs, startup_id, blind);
}

bool
KLauncher::start_service(KService::Ptr service, const QStringList &_urls,
    const QValueList<QCString> &envs, const QCString& startup_id, bool blind, bool autoStart)
{
   QStringList urls = _urls;
   if (!service->isValid())
   {
      DCOPresult.result = ENOEXEC;
      DCOPresult.error = i18n("Service '%1' is malformatted.").arg(service->desktopEntryPath());
      return false;
   }
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = autoStart;

   if ((urls.count() > 1) && !allowMultipleFiles(service))
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
         start_service( service, singleUrl, envs, startup_id, true);
      }
      QString firstURL = *(urls.begin());
      urls.clear();
      urls.append(firstURL);
   }
   createArgs(request, service, urls);

   // We must have one argument at least!
   if (!request->arg_list.count())
   {
      DCOPresult.result = ENOEXEC;
      DCOPresult.error = i18n("Service '%1' is malformatted.").arg(service->desktopEntryPath());
      delete request;
      return false;
   }

   request->name = request->arg_list.first();
   request->arg_list.remove(request->arg_list.begin());

   request->dcop_service_type =  service->DCOPServiceType();

   if ((request->dcop_service_type == KService::DCOP_None) ||
       (request->dcop_service_type == KService::DCOP_Wait))
      request->dcop_name = 0;
   else
      request->dcop_name = request->name;

   request->pid = 0;
   request->transaction = 0;
   request->envs = envs;
   request->startup_id = send_service_startup_info( service, startup_id, envs );

   // Request will be handled later.
   if (!blind && !autoStart)
   {
      request->transaction = dcopClient()->beginTransaction();
   }
   queueRequest(request);
   return true;
}

QCString
KLauncher::send_service_startup_info( KService::Ptr service, const QCString& startup_id,
    const QValueList<QCString> &envs )
{
#ifdef Q_WS_X11 // KStartup* isn't implemented for Qt/Embedded yet
    if( startup_id == "0" )
        return "0";
    QCString wmclass;
    if( service->property( "X-KDE-StartupNotify" ).isValid())
    {
        if( !service->property( "X-KDE-StartupNotify" ).toBool())
            return "0";
        wmclass = service->property( "X-KDE-WMClass" ).toString().latin1();
    }
    else // non-compliant app ( .desktop file )
    {
        if( service->type() != "Application" )
            return "0";
        else
            wmclass = "0";
    }
    KStartupInfoId id;
    id.initId( startup_id );
    static Display* cached_dpy = NULL;
    const char* dpy_str = NULL;
    for( QValueList<QCString>::ConstIterator it = envs.begin();
         it != envs.end();
         ++it )
        if( strncmp( *it, "DISPLAY=", 8 ) == 0 )
            dpy_str = static_cast< const char* >( *it ) + 8;
    Display* dpy = NULL;
    if( dpy_str != NULL && cached_dpy != NULL
        && qstrcmp( dpy_str, XDisplayString( cached_dpy )) == 0 )
        dpy = cached_dpy;
    if( dpy == NULL )
        dpy = XOpenDisplay( dpy_str );
    if( dpy == NULL )
        return id.id();
    KStartupInfoData data;
    data.setName( service->name());
    data.setIcon( service->icon());
    if( !wmclass.isEmpty())
        data.setWMClass( wmclass );
    // the rest will be sent by kdeinit
    KStartupInfo::sendStartupX( dpy, id, data );
    if( cached_dpy != dpy && cached_dpy != NULL )
        XCloseDisplay( cached_dpy );
    cached_dpy = dpy;
    return id.id();
#else
    return 0;
#endif
}

bool
KLauncher::kdeinit_exec(const QString &app, const QStringList &args,
   const QValueList<QCString> &envs,  bool wait)
{
   KLaunchRequest *request = new KLaunchRequest;
   request->autoStart = false;

   for(QStringList::ConstIterator it = args.begin();
       it != args.end();
       it++)
   {
       QString arg = *it;
       request->arg_list.append(arg.local8Bit());
   }

   request->name = app.local8Bit();

   if (wait)
      request->dcop_service_type = KService::DCOP_Wait;
   else
      request->dcop_service_type = KService::DCOP_None;
   request->dcop_name = 0;
   request->pid = 0;
   request->startup_id = "0";
   request->envs = envs;
   request->transaction = dcopClient()->beginTransaction();
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
      KLaunchRequest *request = requestQueue.take(0);
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

bool
KLauncher::allowMultipleFiles(const KService::Ptr service)
{
  QString exec = service->exec();

  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  if ( exec.find( "%F" ) != -1 || exec.find( "%U" ) != -1 || exec.find( "%N" ) != -1 ||
       exec.find( "%D" ) != -1 )
    return true;
  else
    return false;
}

void
KLauncher::createArgs( KLaunchRequest *request, const KService::Ptr service ,
                       const QStringList &urls)
{
  QString exec = service->exec();
  bool b_local_app = false;
  if ( exec.find( "%u" ) == -1 )
    b_local_app = true;

  // Did the user forget to append something like '%f' ?
  // If so, then assume that '%f' is the right choice => the application
  // accepts only local files.
  if ( exec.find( "%f" ) == -1 && exec.find( "%u" ) == -1 && exec.find( "%n" ) == -1 &&
      exec.find( "%d" ) == -1 && exec.find( "%F" ) == -1 && exec.find( "%U" ) == -1 &&
      exec.find( "%N" ) == -1 && exec.find( "%D" ) == -1 )
    exec += " %F";

  // Put args in request->arg_list;
  {
      // This small state machine is used to parse "exec" in order
      // to cut arguments at spaces, but also treat "..." and '...'
      // as a single argument even if they contain spaces. Those simple
      // and double quotes are also removed.
      enum { PARSE_ANY, PARSE_QUOTED, PARSE_DBLQUOTED } state = PARSE_ANY;
      QString arg;
      for ( uint pos = 0; pos < exec.length() ; ++pos )
      {
          QChar ch = exec[pos];
          switch (state) {
              case PARSE_ANY:
                  if ( ch == '\'' && arg.isEmpty() )
                      state = PARSE_QUOTED;
                  else if ( ch == '"' && arg.isEmpty() )
                      state = PARSE_DBLQUOTED;
                  else if ( ch == ' ' )
                  {
                      if (!arg.isEmpty())
                          request->arg_list.append(arg.local8Bit());
                      arg = QString::null;
                      state = PARSE_ANY;
                  }
                  else
                      arg += ch;
                  break;
              case PARSE_QUOTED:
                  if ( ch == '\'' )
                  {
                      request->arg_list.append(arg.local8Bit());
                      arg = QString::null;
                      state = PARSE_ANY;
                  }
                  else
                      arg += ch;
                  break;
              case PARSE_DBLQUOTED:
                  if ( ch == '"' )
                  {
                      request->arg_list.append(arg.local8Bit());
                      arg = QString::null;
                      state = PARSE_ANY;
                  }
                  else
                      arg += ch;
                  break;
          }
      }
      if (!arg.isEmpty())
          request->arg_list.append(arg.local8Bit());
  }

  // Service Name
  replaceArg(request->arg_list, "%c", service->name().local8Bit());

  // Icon
  if (service->icon().isEmpty())
    removeArg(request->arg_list, "%i");
  else
    replaceArg(request->arg_list, "%i", service->icon().local8Bit(), "-icon");

  // Mini-icon
  if (service->icon().isEmpty())
    removeArg(request->arg_list, "%m");
  else
    replaceArg(request->arg_list, "%m", service->icon().local8Bit(), "-miniicon");

  // Desktop-file
  replaceArg(request->arg_list, "%k", QFile::encodeName(service->desktopEntryPath()));

  for(QStringList::ConstIterator it = urls.begin();
      it != urls.end();
      ++it)
  {
    QString url = *it;
    KURL kurl = url;

    QCString f ( QFile::encodeName(kurl.path( -1 )) );
    QCString d ( QFile::encodeName(kurl.directory()) );
    QCString n ( QFile::encodeName(kurl.fileName()) );

    replaceArg(request->arg_list, "%f", f);
    replaceArg(request->arg_list, "%F", f, "%F");

    replaceArg(request->arg_list, "%n", n);
    replaceArg(request->arg_list, "%N", n, "%N");

    replaceArg(request->arg_list, "%d", d);
    replaceArg(request->arg_list, "%D", d, "%D");

    replaceArg(request->arg_list, "%u", url.ascii());
    replaceArg(request->arg_list, "%U", url.ascii(), "%U");
  }

  removeArg(request->arg_list, "%f");
  removeArg(request->arg_list, "%n");
  removeArg(request->arg_list, "%d");
  removeArg(request->arg_list, "%u");
  removeArg(request->arg_list, "%F");
  removeArg(request->arg_list, "%N");
  removeArg(request->arg_list, "%D");
  removeArg(request->arg_list, "%U");
}

void
KLauncher::replaceArg( QValueList<QCString> &args, const QCString &target,
                       const QCString &replace, const char *replacePrefix)
{
   QValueList<QCString>::Iterator it = args.begin();
   while(1) {
      it = args.find( it, target);
      if (it == args.end())
         return;
      if (replacePrefix)
      {
         args.insert(it, QCString(replacePrefix));
      }
      args.insert(it, replace);
      it = args.remove( it );
   }
}

void
KLauncher::removeArg( QValueList<QCString> &args, const QCString &target)
{
   QValueList<QCString>::Iterator it = args.begin();
   while(1) {
      it = args.find( it, target);
      if (it == args.end())
         return;
      it = args.remove( it );
   }
}

///// IO-Slave functions

pid_t
KLauncher::requestHoldSlave(const KURL &url, const QString &app_socket) 
{
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
       if (slave->onHold(url))
          break;
    }
    if (slave)
    {
       mSlaveList.removeRef(slave);
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

    QString _name = KProtocolInfo::exec(protocol);
    if (_name.isEmpty())
    {
	error = i18n("Unknown protocol '%1'.\n").arg(protocol);
        return 0;
    }

    QCString name = _name.latin1(); // ex: "kio_ftp"
    QCString arg1 = protocol.latin1();
    QCString arg2 = QFile::encodeName(mPoolSocketName);
    QCString arg3 = QFile::encodeName(app_socket);
    QValueList<QCString> arg_list;
    arg_list.append(arg1);
    arg_list.append(arg2);
    arg_list.append(arg3);

//    kdDebug(7016) << "KLauncher: launching new slave " << _name << " with protocol=" << protocol << endl;
    if (mSlaveDebug == arg1)
    {
       klauncher_header request_header;
       request_header.cmd = LAUNCHER_DEBUG_WAIT;
       request_header.arg_length = 0;
       write(kdeinitSocket, &request_header, sizeof(request_header));
    }

    KLaunchRequest *request = new KLaunchRequest;
    request->autoStart = false;
    request->name = name;
    request->arg_list =  arg_list;
    request->dcop_name = 0;
    request->dcop_service_type = KService::DCOP_None;
    request->pid = 0;
    request->startup_id = "0";
    request->status = KLaunchRequest::Launching;
    request->transaction = 0; // No confirmation is send
    requestStart(request);
    pid_t pid = request->pid;

//    kdDebug(7016) << "Slave launched, pid = " << pid << endl;

    // We don't care about this request any longer....
    requestDone(request);
    if (!pid)
    {
       error = i18n("Error loading '%1'.\n").arg(name);
    }
    return pid;
}

void 
KLauncher::waitForSlave(pid_t pid)
{
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
        if (slave->pid() == pid)
           return; // Already here.
    }
    SlaveWaitRequest *waitRequest = new SlaveWaitRequest;
    waitRequest->transaction = dcopClient()->beginTransaction();
    waitRequest->pid = pid;
    mSlaveWaitRequest.append(waitRequest);    
}

void
KLauncher::acceptSlave(KSocket *slaveSocket)
{
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
    for(SlaveWaitRequest *waitRequest=mSlaveWaitRequest.first();
        waitRequest; waitRequest = mSlaveWaitRequest.next())
    {
       if (waitRequest->pid == slave->pid())
       {
          QByteArray replyData;
          QCString replyType;
          replyType = "void";
          dcopClient()->endTransaction( waitRequest->transaction, replyType, replyData);
          mSlaveWaitRequest.removeRef(waitRequest);
          waitRequest = mSlaveWaitRequest.current();
       }
       else
       {
          waitRequest = mSlaveWaitRequest.next();
       }
    }
}

void
KLauncher::slotSlaveGone()
{
    IdleSlave *slave = (IdleSlave *) sender();
    mSlaveList.removeRef(slave);
    if ((mSlaveList.count() == 0) && (mTimer.isActive()))
    {
       mTimer.stop();
    }
}

void
KLauncher::idleTimeout()
{
    time_t now = time(0);
    IdleSlave *slave;
    for(slave = mSlaveList.first(); slave; slave = mSlaveList.next())
    {
        if (slave->age(now) > SLAVE_MAX_IDLE)
        {
           // killing idle slave
           delete slave;
        }
    }
}

#include "klauncher.moc"
