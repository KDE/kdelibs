/* This file is part of the KDE libraries
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "config.h"

#include <dcopclient.h>
#include <assert.h>
#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>

#include "kuniqueapp.moc"

DCOPClient *KUniqueApplication::s_DCOPClient = 0;

DCOPClient *
KUniqueApplication::dcopClient()
{
  assert( s_DCOPClient);
  return s_DCOPClient;
}

bool
KUniqueApplication::start(int& argc, char** argv,
                          const QCString& rAppName)
{
  bool nofork = false;
  for(int i = 1; i < argc; i++)
  {
     if (strcmp(argv[i], "--nofork") == 0)
        nofork = true;
  }
  if (nofork)
  {
     s_DCOPClient = new DCOPClient();
     s_DCOPClient->registerAs(rAppName);
     return true;
  }     
  DCOPClient *dc;
  int fd[2];
  char result;
  if (0 > pipe(fd))
  {
     qDebug("KUniqueApplication: pipe() failed!\n");
     ::exit(255);
  }
  switch(fork()) {
  case -1:
     qDebug("KUniqueApplication: fork() failed!\n");
     ::exit(255);
     break;
  case 0:
     // Child
     ::close(fd[0]);
     dc = new DCOPClient();
     {
        QCString regName = dc->registerAs(rAppName);
        if (regName.isEmpty())
        {
           qDebug("KUniqueApplication: Child can't attach to DCOP.\n");
           result = -1;
           delete dc;	// Clean up DCOP commmunication
           ::write(fd[1], &result, 1);
           ::exit(255);
        }
        if (regName != rAppName) 
        {
           // Already running. Ok.
           result = 0;
           delete dc;	// Clean up DCOP commmunication
           ::write(fd[1], &result, 1);
           ::close(fd[1]);
           return false;
        }
     }
     
     s_DCOPClient = dc;
     result = 0;
     ::write(fd[1], &result, 1);
     ::close(fd[1]);
     return true; // Finished.
     break;
  default:
     // Parent
     ::close(fd[1]);
     for(;;)
     {
       int n = ::read(fd[0], &result, 1);
       if (n == 1) break;
       if (n == 0)
       {
          qDebug("KUniqueApplication: Pipe closed unexpected.\n");
          ::exit(255);
       }
       if (errno != EINTR)
       {
          qDebug("KUniqueApplication: Error reading from pipe.\n");
          ::exit(255);
       }
     }
     ::close(fd[0]);

     if (result != 0)
        ::exit(result); // Error occured in child.

     dc = new DCOPClient();
     if (!dc->attach())
     {
        qDebug("KUniqueApplication: Parent can't attach to DCOP.\n");
        delete dc;	// Clean up DCOP commmunication
        ::exit(255);
     }
     if (!dc->isApplicationRegistered(rAppName)) {
        qDebug("KUniqueApplication: Registering failed!\n");
     }
     QByteArray data, reply;
     QDataStream ds(data, IO_WriteOnly);
     QValueList<QCString> params;
     for (int i = 0; i < argc; i++)
        params.append(argv[i]);

     ds << params;
     QCString replyType;
     if (!dc->call(rAppName, rAppName, "newInstance(QValueList<QCString>)", data, replyType, reply))
     {
        qDebug("KUniqueApplication: DCOP communication error!");
        delete dc;	// Clean up DCOP commmunication
        ::exit(255);
     }
     if (replyType != "int")
     {
        qDebug("KUniqueApplication: DCOP communication error!"); 
        delete dc;	// Clean up DCOP commmunication
        ::exit(255);
     }
     QDataStream rs(reply, IO_ReadOnly);
     int exitCode;
     rs >> exitCode;
     delete dc;	// Clean up DCOP commmunication
     ::exit(exitCode);
     break;
  }
}


KUniqueApplication::KUniqueApplication(int& argc, char** argv,
				       const QCString& rAppName,
                                       bool allowStyles)
  : KApplication(argc, argv, rAppName, allowStyles), DCOPObject(rAppName)
{
  if (!s_DCOPClient)
  {
     if (!start(argc, argv, rAppName))
     {
         // Already running
         ::exit(0);
     }
  }
  s_DCOPClient->bindToApp(); // Make sure we get events from the DCOPClient.
}

KUniqueApplication::~KUniqueApplication()
{
}

bool KUniqueApplication::process(const QCString &fun, const QByteArray &data,
				 QCString &replyType, QByteArray &replyData)
{
  if (fun == "newInstance(QValueList<QCString>)") {
    QDataStream ds(data, IO_ReadOnly);
    QValueList<QCString> params;
    ds >> params;
    int exitCode = newInstance(params);
    QDataStream rs(replyData, IO_WriteOnly);
    rs << exitCode;
    replyType = "int";
    return true;
  } else
    return false;
}

int KUniqueApplication::newInstance(QValueList<QCString> /*params*/)
{
  return 0; // do nothing in default implementation
}
