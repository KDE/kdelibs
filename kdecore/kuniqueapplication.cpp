/* This file is part of the KDE libraries
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>

    $Id$

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

#include <config.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <qfile.h>
#include <qptrlist.h>
#include <qtimer.h>

#include <dcopclient.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <kwin.h> 
#include <kstartupinfo.h> 
#endif

#include <kconfig.h>
#include "kdebug.h"
#include "kuniqueapplication.h"

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <netwm.h> 
#include <X11/Xlib.h> 
#define DISPLAY "DISPLAY"
#else
#  ifdef Q_WS_QWS
#    define DISPLAY "QWS_DISPLAY"
#  else
#    define DISPLAY "DISPLAY"
#  endif
#endif

bool KUniqueApplication::s_nofork = false;
bool KUniqueApplication::s_multipleInstances = false;
bool KUniqueApplication::s_uniqueTestDone = false;

static KCmdLineOptions kunique_options[] =
{
  { "nofork", "Don't run in the background.", 0 },
  KCmdLineLastOption
};

struct DCOPRequest {
   QCString fun;
   QByteArray data;
   DCOPClientTransaction *transaction;
};

class KUniqueApplicationPrivate {
public:
   QPtrList <DCOPRequest> requestList;
   bool processingRequest;
   bool firstInstance;
};

void
KUniqueApplication::addCmdLineOptions()
{
  KCmdLineArgs::addCmdLineOptions(kunique_options, 0, "kuniqueapp", "kde" );
}

bool
KUniqueApplication::start()
{
  if( s_uniqueTestDone )
    return true;
  s_uniqueTestDone = true;
  addCmdLineOptions(); // Make sure to add cmd line options
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kuniqueapp");
  s_nofork = !args->isSet("fork");
  delete args;

  QCString appName = KCmdLineArgs::about->appName();

  if (s_nofork)
  {
     if (s_multipleInstances)
     {
        QCString pid;
        pid.setNum(getpid());
        appName = appName + "-" + pid;
     }

     // Check to make sure that we're actually able to register with the DCOP
     // server.

     if(dcopClient()->registerAs(appName, false).isEmpty()) {
        startKdeinit();
        if(dcopClient()->registerAs(appName, false).isEmpty()) {
           kdError() << "KUniqueApplication: Can't setup DCOP communication." << endl;
           ::exit(255);
        }           
     }

     // We'll call newInstance in the constructor. Do nothing here.
     return true;
  }
  DCOPClient *dc;
  int fd[2];
  signed char result;
  if (0 > pipe(fd))
  {
     kdError() << "KUniqueApplication: pipe() failed!" << endl;
     ::exit(255);
  }
  int fork_result = fork();
  switch(fork_result) {
  case -1:
     kdError() << "KUniqueApplication: fork() failed!" << endl;
     ::exit(255);
     break;
  case 0:
     // Child
     ::close(fd[0]);
     if (s_multipleInstances)
        appName.append("-").append(QCString().setNum(getpid()));
     dc = dcopClient();
     {
        QCString regName = dc->registerAs(appName, false);
        if (regName.isEmpty())
        {
           // Check DISPLAY
           if (QCString(getenv(DISPLAY)).isEmpty())
           {
              kdError() << "KUniqueApplication: Can't determine DISPLAY. Aborting." << endl;
              result = -1; // Error
              ::write(fd[1], &result, 1);
              ::exit(255);
           }

           // Try to launch kdeinit.
           startKdeinit();
           regName = dc->registerAs(appName, false);
           if (regName.isEmpty())
           {
              kdError() << "KUniqueApplication: Can't setup DCOP communication." << endl;
              result = -1;
              delete dc;	// Clean up DCOP commmunication
              ::write(fd[1], &result, 1);
              ::exit(255);
           }
        }
        if (regName != appName)
        {
           // Already running. Ok.
           result = 0;
           delete dc;	// Clean up DCOP commmunication
           ::write(fd[1], &result, 1);
           ::close(fd[1]);
#if 0
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
//#ifdef Q_WS_X11
           // say we're up and running ( probably no new window will appear )
           KStartupInfoId id;
           if( kapp != NULL ) // KApplication constructor unsets the env. variable
               id.initId( kapp->startupId());
           else
               id = KStartupInfo::currentStartupIdEnv();
           if( !id.none())
           {
               Display* disp = XOpenDisplay( NULL );
               if( disp != NULL ) // use extra X connection
               {
                   KStartupInfo::sendFinishX( disp, id );
                   XCloseDisplay( disp );
               }
           }
#else //FIXME(E): implement
#endif
#endif
           return false;
        }
        dc->setPriorityCall(true);
     }

     {
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
//#ifdef Q_WS_X11
         KStartupInfoId id;
         if( kapp != NULL ) // KApplication constructor unsets the env. variable
             id.initId( kapp->startupId());
         else
             id = KStartupInfo::currentStartupIdEnv();
         if( !id.none())
         { // notice about pid change
            Display* disp = XOpenDisplay( NULL );
            if( disp != NULL ) // use extra X connection
               {
               KStartupInfoData data;
               data.addPid( getpid());
               KStartupInfo::sendChangeX( disp, id, data );
               XCloseDisplay( disp );
               }
         }
#else //FIXME(E): Implement
#endif
     }
     result = 0;
     ::write(fd[1], &result, 1);
     ::close(fd[1]);
     return true; // Finished.
  default:
     // Parent
//     DCOPClient::emergencyClose();
//     dcopClient()->detach();
     if (s_multipleInstances)
        appName.append("-").append(QCString().setNum(fork_result));
     ::close(fd[1]);
     for(;;)
     {
       int n = ::read(fd[0], &result, 1);
       if (n == 1) break;
       if (n == 0)
       {
          kdError() << "KUniqueApplication: Pipe closed unexpectedly." << endl;
          ::exit(255);
       }
       if (errno != EINTR)
       {
          kdError() << "KUniqueApplication: Error reading from pipe." << endl;
          ::exit(255);
       }
     }
     ::close(fd[0]);

     if (result != 0)
        ::exit(result); // Error occurred in child.

     dc = new DCOPClient();
     if (!dc->attach())
     {
        kdError() << "KUniqueApplication: Parent process can't attach to DCOP." << endl;
        delete dc;	// Clean up DCOP commmunication
        ::exit(255);
     }
     if (!dc->isApplicationRegistered(appName)) {
        kdError() << "KUniqueApplication: Registering failed!" << endl;
     }

     QCString new_asn_id;
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
     KStartupInfoId id;
     if( kapp != NULL ) // KApplication constructor unsets the env. variable
         id.initId( kapp->startupId());
     else
         id = KStartupInfo::currentStartupIdEnv();
     if( !id.none())
         new_asn_id = id.id();
#endif
     
     QByteArray data, reply;
     QDataStream ds(data, IO_WriteOnly);

     KCmdLineArgs::saveAppArgs(ds);
     ds << new_asn_id;

     dc->setPriorityCall(true);
     QCString replyType;
     if (!dc->call(appName, KCmdLineArgs::about->appName(), "newInstance()", data, replyType, reply))
     {
        kdError() << "Communication problem with " << KCmdLineArgs::about->appName() << ", it probably crashed." << endl;
        delete dc;	// Clean up DCOP commmunication
        ::exit(255);
     }
     dc->setPriorityCall(false);
     if (replyType != "int")
     {
        kdError() << "KUniqueApplication: DCOP communication error!" << endl;
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
  return false; // make insure++ happy
}


KUniqueApplication::KUniqueApplication(bool allowStyles, bool GUIenabled, bool configUnique)
  : KApplication( allowStyles, GUIenabled, initHack( configUnique )),
    DCOPObject(KCmdLineArgs::about->appName())
{
  d = new KUniqueApplicationPrivate;
  d->processingRequest = false;
  d->firstInstance = true;

  if (s_nofork)
    // Can't call newInstance directly from the constructor since it's virtual...
    QTimer::singleShot( 0, this, SLOT(newInstanceNoFork()) );
}

KUniqueApplication::~KUniqueApplication()
{
  delete d;
}

// this gets called before even entering QApplication::QApplication()
KInstance* KUniqueApplication::initHack( bool configUnique )
{
  KInstance* inst = new KInstance( KCmdLineArgs::about );
  if (configUnique)
  {
    KConfigGroupSaver saver( inst->config(), "KDE" );
    s_multipleInstances = inst->config()->readBoolEntry("MultipleInstances", false);
  }
  if( !start())
         // Already running
      ::exit( 0 );
  return inst;
}

void KUniqueApplication::newInstanceNoFork()
{
  if (dcopClient()->isSuspended())
  {
    // Try again later.
    QTimer::singleShot( 200, this, SLOT(newInstanceNoFork()) );
    return;
  }
  
  newInstance();
  d->firstInstance = false;
  KStartupInfo::handleAutoAppStartedSending(); // KDE4 remove?
  // What to do with the return value ?
}

bool KUniqueApplication::process(const QCString &fun, const QByteArray &data,
				 QCString &replyType, QByteArray &replyData)
{
  if (fun == "newInstance()")
  {
    delayRequest(fun, data);
    return true;
  } else
    return DCOPObject::process(fun, data, replyType, replyData);
}

void
KUniqueApplication::delayRequest(const QCString &fun, const QByteArray &data)
{
  DCOPRequest *request = new DCOPRequest;
  request->fun = fun;
  request->data = data;
  request->transaction = dcopClient()->beginTransaction();
  d->requestList.append(request);
  if (!d->processingRequest)
  {
     QTimer::singleShot(0, this, SLOT(processDelayed()));
  }
}

void
KUniqueApplication::processDelayed()
{
  if (dcopClient()->isSuspended())
  {
    // Try again later.
    QTimer::singleShot( 200, this, SLOT(processDelayed()));
    return;
  }
  d->processingRequest = true;
  while( !d->requestList.isEmpty() )
  {
     DCOPRequest *request = d->requestList.take(0);
     QByteArray replyData;
     QCString replyType;
     if (request->fun == "newInstance()") {
       dcopClient()->setPriorityCall(false);
       QDataStream ds(request->data, IO_ReadOnly);
       KCmdLineArgs::loadAppArgs(ds);
       if( !ds.atEnd()) // backwards compatibility
       {
           QCString asn_id;
           ds >> asn_id;
           setStartupId( asn_id );
       }
       int exitCode = newInstance();
       d->firstInstance = false;
       KStartupInfo::handleAutoAppStartedSending(); // KDE4 remove?
       QDataStream rs(replyData, IO_WriteOnly);
       rs << exitCode;
       replyType = "int";
     }
     dcopClient()->endTransaction( request->transaction, replyType, replyData);
     delete request;
  }

  d->processingRequest = false;
}

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
//#ifndef Q_WS_QWS // FIXME(E): Implement for Qt/Embedded
extern Time qt_x_time;
#endif

bool KUniqueApplication::restoringSession()
{
  return d->firstInstance && isRestored();
}

int KUniqueApplication::newInstance()
{
  if (!d->firstInstance)
  {
//#ifndef Q_WS_QWS // FIXME(E): Implement for Qt/Embedded
    if ( mainWidget() )
    {
      mainWidget()->show();
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    // This is the line that handles window activation if necessary,
    // and what's important, it does it properly. If you reimplement newInstance(),
    // and don't call the inherited one, use this.
      KStartupInfo::setNewStartupId( mainWidget(), kapp->startupId());
#endif
    }
  }
  return 0; // do nothing in default implementation
}

void KUniqueApplication::virtual_hook( int id, void* data )
{ KApplication::virtual_hook( id, data );
  DCOPObject::virtual_hook( id, data ); }

#include "kuniqueapplication.moc"
