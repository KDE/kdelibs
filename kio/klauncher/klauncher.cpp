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

#include "klauncher.h"
#include "klauncher_cmds.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

LaunchApp::LaunchApp(int argc, char **argv, const QCString &appName, 
                     int _kinitSocket)
  : KUniqueApplication(argc, argv, appName, false), 
    kinitSocket(_kinitSocket)
{
   requestList.setAutoDelete(true);
   dcopClient()->setNotifications( true );
   connect(dcopClient(), SIGNAL( applicationRegistered( const QCString &)),
           this, SLOT( slotAppRegistered( const QCString &)));

   kinitNotifier = new QSocketNotifier(kinitSocket, QSocketNotifier::Read);
   connect(kinitNotifier, SIGNAL( activated( int )),
           this, SLOT( slotKInitData( int )));
}

bool
LaunchApp::process(const QCString &fun, const QByteArray &data,
                   QCString &replyType, QByteArray &replyData)
{
   if (fun == "exec_blind(QCString,QValueList<QCString>)")
   {
      QDataStream stream(data, IO_ReadOnly);
      
      KLaunchRequest *request = new KLaunchRequest;
      stream >> request->name >> request->arg_list;
fprintf(stderr, "KLauncher: Got exec_blind('%s', ...)\n", request->name.data());
      request->dcop_name = request->name;
      request->pid = 0;
      request->status = KLaunchRequest::Launching;
      request->transaction = 0; // No confirmation is send
      requestStart(request);
      return true;
   }   
   else if (KUniqueApplication::process(fun, data, replyType, replyData))
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
LaunchApp::slotKInitData(int)
{
   klauncher_header request_header;
   QByteArray requestData;
   int result = read_socket(kinitSocket, (char *) &request_header, 
                            sizeof( request_header));
   if (result == -1)
   {
      fprintf(stderr, "KLauncher: KInit communication error! Commiting suicide!\n");
      ::exit(255);
   }
   requestData.resize(request_header.arg_length);
   result = read_socket(kinitSocket, (char *) requestData.data(), 
                        request_header.arg_length);

   if (request_header.cmd == LAUNCHER_DIED)
   {
     long *request_data;
     request_data = (long *) requestData.data();
     processDied(request_data[0], request_data[1]);
     return;
   }
   fprintf(stderr, "KLauncher: Unexpected command from KInit (%d)\n", 
	   request_header.cmd);
}

void
LaunchApp::processDied(pid_t pid, long exitStatus)
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
LaunchApp::slotAppRegistered(const QCString &appId)
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
LaunchApp::requestDone(KLaunchRequest *request)
{
   fprintf(stderr, "KLauncher: Request done [ name = '%s' , status ='%s' ]\n",
 	   request->name.data(), 
           request->status == KLaunchRequest::Running ? "running" : "error");
   requestList.removeRef( request );
}

void 
LaunchApp::requestStart(KLaunchRequest *request)
{
   fprintf(stderr, "KLauncher: Request start [ name = '%s' ]\n",
 	   request->name.data());
   requestList.append( request );
   // Send request to kinit.
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
   *((long *)p)++ = request->arg_list.count()+1;
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
   write(kinitSocket, &request_header, sizeof(request_header));
   write(kinitSocket, requestData.data(), request_header.arg_length);

   // Wait for pid to return.
   int result = read_socket(kinitSocket, (char *) &request_header, 
                            sizeof( request_header));
   if (result == -1)
   {
      fprintf(stderr, "KLauncher: KInit communication error! Commiting suicide!\n");
      ::exit(255);
   }
   requestData.resize(request_header.arg_length);
   result = read_socket(kinitSocket, (char *) requestData.data(), 
                        request_header.arg_length);

   if (request_header.cmd == LAUNCHER_OK)
   {
     long *request_data;
     request_data = (long *) requestData.data();
     request->pid = (pid_t) (*request_data);
     request->status = KLaunchRequest::Launching;
     // Start timer
     return;
   }
   if (request_header.cmd == LAUNCHER_ERROR)
   {
     request->status = KLaunchRequest::Error;
     requestDone(request);
     return; 
   }
   fprintf(stderr, "KLauncher: Unexpected command from KInit (%d)\n", 
	   request_header.cmd);

}

#include "klauncher.moc"
