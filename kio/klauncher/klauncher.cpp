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

#include <qfile.h>

#include <klocale.h>
#include <kurl.h>

#include "klauncher.h"
#include "klauncher_cmds.h"

template class QList<KLaunchRequest>;

KLauncher::KLauncher(int _kdeinitSocket)
  : KUniqueApplication( false, false ), // No Styles, No GUI
    kdeinitSocket(_kdeinitSocket)
{
   requestList.setAutoDelete(true);
   dcopClient()->setNotifications( true );
   connect(dcopClient(), SIGNAL( applicationRegistered( const QCString &)),
           this, SLOT( slotAppRegistered( const QCString &)));

   kdeinitNotifier = new QSocketNotifier(kdeinitSocket, QSocketNotifier::Read);
   connect(kdeinitNotifier, SIGNAL( activated( int )),
           this, SLOT( slotKInitData( int )));
   kdeinitNotifier->setEnabled( true );
   lastRequest = 0;
}

bool
KLauncher::process(const QCString &fun, const QByteArray &data,
                   QCString &replyType, QByteArray &replyData)
{
   if (fun == "exec_blind(QCString,QValueList<QCString>)")
   {
      QDataStream stream(data, IO_ReadOnly);

      QCString name;
      QValueList<QCString> arg_list;      
      stream >> name >> arg_list;
fprintf(stderr, "KLauncher: Got exec_blind('%s', ...)\n", name.data());
      exec_blind( name, arg_list);
      return true;
   }   
   if ((fun == "start_service_by_name(QString,QString)") ||
       (fun == "start_service_by_desktop_path(QString,QString)")||
       (fun == "start_service_by_desktop_name(QString,QString)"))
   {
      QDataStream stream(data, IO_ReadOnly);
      
      QString serviceName;
      QString filename;
      DCOPresult.result = -1;
      DCOPresult.dcopName = 0;
      DCOPresult.error = QString::null;
      stream >> serviceName >> filename;
      bool finished;
      if (fun == "start_service_by_name(QString,QString)")
      {
fprintf(stderr, "KLauncher: Got start_service_by_name('%s', ...)\n", serviceName.data());
         finished = start_service_by_name(serviceName, filename);
      }
      else if (fun == "start_service_by_desktop_path(QString,QString)")
      {
fprintf(stderr, "KLauncher: Got start_service_by_desktop_path('%s', ...)\n", serviceName.data());
         finished = start_service_by_desktop_path(serviceName, filename);
      }
      else 
      {
fprintf(stderr, "KLauncher: Got start_service_by_desktop_name('%s', ...)\n", serviceName.data());
         finished = start_service_by_desktop_name(serviceName, filename);
      }
      if (!finished)
      {
         replyType = "serviceResult";
         QDataStream stream2(replyData, IO_WriteOnly);
         stream2 << DCOPresult.result << DCOPresult.dcopName << DCOPresult.error;
      }
      return true;
   }
   if (KUniqueApplication::process(fun, data, replyType, replyData))
   {
      return true;
   }
fprintf(stderr, "KLauncher: Got unknown DCOP function '%s'\n", fun.data());   
   return false;
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
KLauncher::slotKInitData(int)
{
   klauncher_header request_header;
   QByteArray requestData;
   fprintf(stderr, "KLauncher: Data from KInit!\n");
   int result = read_socket(kdeinitSocket, (char *) &request_header, 
                            sizeof( request_header));
   if (result == -1)
   {
      fprintf(stderr, "KLauncher: KInit communication error! Commiting suicide!\n");
      ::exit(255);
   }
   requestData.resize(request_header.arg_length);
   result = read_socket(kdeinitSocket, (char *) requestData.data(), 
                        request_header.arg_length);

   fprintf(stderr, "KLauncher: Got notification (%ld) from KInit\n", 
	request_header.cmd);
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
     fprintf(stderr, "KLauncher: %s (pid %d) up and running.\n", 
               lastRequest->name.data(), lastRequest->pid);
     switch(lastRequest->dcop_service_type)
     {
       case KService::DCOP_None:
     fprintf(stderr, "KLauncher: DCOP_None, ready.\n");
         lastRequest->status = KLaunchRequest::Running;
         break;

       case KService::DCOP_Unique:
     fprintf(stderr, "KLauncher: DCOP_Unique, waiting for '%s' to register.\n", 
		lastRequest->dcop_name.data());
         lastRequest->status = KLaunchRequest::Launching;
         break;

       case KService::DCOP_Multi:
         lastRequest->status = KLaunchRequest::Launching;
         QCString pidStr;
         pidStr.setNum(lastRequest->pid);
         lastRequest->dcop_name = lastRequest->name + "-" + pidStr;
     fprintf(stderr, "KLauncher: DCOP_Multi, waiting for '%s' to register.\n", 
		lastRequest->dcop_name.data());
         break;
     }
     lastRequest = 0;
     return;
   }
   if (lastRequest && (request_header.cmd == LAUNCHER_ERROR))
   {
     lastRequest->status = KLaunchRequest::Error;
     lastRequest = 0;
     return; 
   }
   
   fprintf(stderr, "KLauncher: Unexpected command from KInit (%ld)\n", 
	   request_header.cmd);
}

void
KLauncher::processDied(pid_t pid, long /* exitStatus */)
{
   KLaunchRequest *request = requestList.first();
   for(; request; request = requestList.next())
   {
      if (request->pid == pid)
      {
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
KLauncher::requestDone(KLaunchRequest *request)
{
   fprintf(stderr, "KLauncher: Request done [ name = '%s' , status ='%s' ]\n",
 	   request->name.data(), 
           request->status == KLaunchRequest::Running ? "running" : "error");
   if (request->status == KLaunchRequest::Running)
   {
      DCOPresult.result = 0;
      DCOPresult.dcopName = request->dcop_name;
      DCOPresult.error = QString::null;
   }
   else 
   {
      DCOPresult.result = 1;
      DCOPresult.dcopName = "";
      DCOPresult.error = i18n("KInit could not launch '%1'").arg(request->name);
   }
   if (request->transaction)
   {
      QByteArray replyData;
      QCString replyType;
      replyType = "serviceResult";
      QDataStream stream2(replyData, IO_WriteOnly);
      stream2 << DCOPresult.result << DCOPresult.dcopName << DCOPresult.error;
      dcopClient()->endTransaction( request->transaction, 
                                    replyType, replyData);
      requestList.removeRef( request );
   }
}

void 
KLauncher::requestStart(KLaunchRequest *request)
{
   fprintf(stderr, "KLauncher: Request start [ name = '%s' ]\n",
 	   request->name.data());
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
   requestData.resize( length );
   
   char *p = requestData.data();
   *(reinterpret_cast<long *>(p)) = request->arg_list.count()+1;
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
fprintf(stderr, "args = %d arg_length = %d\n", request->arg_list.count()+1, length);
   
   request_header.cmd = LAUNCHER_EXEC;
   request_header.arg_length = length;
   write(kdeinitSocket, &request_header, sizeof(request_header));
   write(kdeinitSocket, requestData.data(), request_header.arg_length);

   // Wait for pid to return.
   lastRequest = request;
   do {
      slotKInitData( kdeinitSocket );
   } 
   while (lastRequest != 0);
}

void
KLauncher::exec_blind( const QCString &name, const QValueList<QCString> &arg_list)
{
   KLaunchRequest *request = new KLaunchRequest;
   request->name = name;
   request->arg_list =  arg_list;
   request->dcop_name = QString::null;
   request->dcop_service_type = KService::DCOP_None;
   request->pid = 0;
   request->status = KLaunchRequest::Launching;
   request->transaction = 0; // No confirmation is send
   requestStart(request);
   // We don't care about this request any longer....
   requestDone(request);
   requestList.removeRef( request ); 
}


bool
KLauncher::start_service_by_name(const QString &serviceName, const QString &filename)
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
   return start_service(service, filename);
}

bool
KLauncher::start_service_by_desktop_path(const QString &serviceName, const QString &filename)
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
   return start_service(service, filename);
}

bool
KLauncher::start_service_by_desktop_name(const QString &serviceName, const QString &filename)
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
   return start_service(service, filename);
}

bool 
KLauncher::start_service(KService::Ptr service, const QString &filename)
{
   if (!service->isValid())
   {
      DCOPresult.result = ENOEXEC;
      DCOPresult.error = i18n("Service '%1' is malformatted.").arg(service->desktopEntryPath());
      return false;
   }
   KLaunchRequest *request = new KLaunchRequest;

   createArgs(request, service, filename);

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

   if (request->dcop_service_type == KService::DCOP_None)
      request->dcop_name = QString::null;
   else
      request->dcop_name = request->name;
   
   request->pid = 0;
   request->transaction = 0;
   // Are we already running?
   if (request->dcop_service_type == KService::DCOP_Unique)
   {
      if (dcopClient()->isApplicationRegistered(request->dcop_name))
      {
         // Yes, service is already running.
         request->status = KLaunchRequest::Running;
         // Request handled.
         requestDone( request );
         requestList.removeRef( request );
         return false;
      }
   }

   request->status = KLaunchRequest::Launching;
   requestStart(request);
   if (request->status == KLaunchRequest::Launching)
   {
      // Request will be handled later.
      request->transaction = dcopClient()->beginTransaction();
      return true;
   }

   // Request handled.
   requestDone( request );
   requestList.removeRef( request );
   return false;
}

void
KLauncher::createArgs( KLaunchRequest *request, const KService::Ptr service ,
                       const QString &url)
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
    exec += " %f";

#if 0
  // Can we pass multiple files on the command line or do we have to start the application for every single file ?
  bool b_allow_multiple = false;
  if ( exec.find( "%F" ) != -1 || exec.find( "%U" ) != -1 || exec.find( "%N" ) != -1 ||
       exec.find( "%D" ) != -1 )
    b_allow_multiple = true;
#endif

  // Put args in request->arg_list;
  {
     QStringList args = QStringList::split( ' ', exec);
     for(QStringList::Iterator it = args.begin();
         it != args.end();
         it++)
     {
         QString arg = *it;
         // Unquote.
         if ((arg.length() > 1) && 
             ((arg[0] == '\"') && (arg[arg.length()-1] == '\"') ||
              (arg[0] == '\'') && (arg[arg.length()-1] == '\''))
            )
         {
            arg = arg.mid(1, arg.length()-2);
         }
         request->arg_list.append(QCString(arg.ascii()));
     }
  }

  // Service Name
  replaceArg(request->arg_list, "%c", service->name().ascii());
  
  // Icon
  if (service->icon().isEmpty())
    removeArg(request->arg_list, "%i");
  else
    replaceArg(request->arg_list, "%i", service->icon().ascii(), "-icon");

  // Mini-icon
  if (service->icon().isEmpty())
    removeArg(request->arg_list, "%m");
  else
    replaceArg(request->arg_list, "%m", service->icon().ascii(), "-miniicon");

  // Desktop-file
  replaceArg(request->arg_list, "%k", QFile::encodeName(service->desktopEntryPath()));

  if (url.isEmpty())
  {
    removeArg(request->arg_list, "%f");
    removeArg(request->arg_list, "%n");
    removeArg(request->arg_list, "%d");
    removeArg(request->arg_list, "%u");
    removeArg(request->arg_list, "%F");
    removeArg(request->arg_list, "%N");
    removeArg(request->arg_list, "%D");
    removeArg(request->arg_list, "%U");
  }

  KURL kurl = url;
  QCString f ( QFile::encodeName(kurl.path( -1 )) );
  QCString d ( QFile::encodeName(kurl.directory()) );
  QCString n ( QFile::encodeName(kurl.filename()) );

  replaceArg(request->arg_list, "%f", f);
  replaceArg(request->arg_list, "%F", f);

  replaceArg(request->arg_list, "%n", n);
  replaceArg(request->arg_list, "%N", n);

  replaceArg(request->arg_list, "%d", d);
  replaceArg(request->arg_list, "%D", d);

  replaceArg(request->arg_list, "%u", url.ascii());
  replaceArg(request->arg_list, "%U", url.ascii());
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

#include "klauncher.moc"
