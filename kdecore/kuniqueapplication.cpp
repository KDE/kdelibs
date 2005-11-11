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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <qfile.h>
#include <qlist.h>
#include <qtimer.h>

#include <dcopclient.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

#if defined Q_WS_X11
#include <kwin.h>
#include <kstartupinfo.h>
#endif

#include <kconfig.h>
#include "kdebug.h"
#include "kuniqueapplication.h"

#if defined Q_WS_X11
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
bool KUniqueApplication::s_handleAutoStarted = false;

static KCmdLineOptions kunique_options[] =
{
  { "nofork", "Don't run in the background.", 0 },
  KCmdLineLastOption
};

struct DCOPRequest {
   QByteArray fun;
   QByteArray data;
   DCOPClientTransaction *transaction;
};

class KUniqueApplication::Private
{
public:
   QList <DCOPRequest *> requestList;
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
#ifdef Q_WS_WIN
  s_nofork = true;
#else
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kuniqueapp");
  s_nofork = !args->isSet("fork");
  delete args;
#endif

  QByteArray appName = KCmdLineArgs::about->appName();

  if (s_nofork)
  {
     if (s_multipleInstances)
     {
        QByteArray pid;
        pid.setNum(getpid());
        appName = appName + "-" + pid;
     }

     // Check to make sure that we're actually able to register with the DCOP
     // server.

#ifndef Q_WS_WIN //TODO
     if(dcopClient()->registerAs(appName, false).isEmpty()) {
        startKdeinit();
        if(dcopClient()->registerAs(appName, false).isEmpty()) {
           kdError() << "KUniqueApplication: Can't setup DCOP communication." << endl;
           ::exit(255);
        }
     }
#endif

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
        appName.append("-").append(QByteArray().setNum(getpid()));
     dc = dcopClient();
     {
        QByteArray regName = dc->registerAs(appName, false);
        if (regName.isEmpty())
        {
           // Check DISPLAY
           if (QByteArray(getenv(DISPLAY)).isEmpty())
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
#ifdef Q_WS_X11
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
#ifdef Q_WS_X11
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
        appName.append("-").append(QByteArray().setNum(fork_result));
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

     QByteArray new_asn_id;
#if defined Q_WS_X11
     KStartupInfoId id;
     if( kapp != NULL ) // KApplication constructor unsets the env. variable
         id.initId( kapp->startupId());
     else
         id = KStartupInfo::currentStartupIdEnv();
     if( !id.none())
         new_asn_id = id.id();
#endif

     QByteArray data, reply;
     QDataStream ds(&data, QIODevice::WriteOnly);
	 ds.setVersion( QDataStream::Qt_3_1 );

     KCmdLineArgs::saveAppArgs(ds);
     ds << new_asn_id;

     dc->setPriorityCall(true);
     DCOPCString replyType;
     if (!dc->call( DCOPCString( appName ), DCOPCString( KCmdLineArgs::about->appName() ), "newInstance()", data, replyType, reply))
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
     QDataStream rs(reply);
     int exitCode;
     rs >> exitCode;
     delete dc;	// Clean up DCOP commmunication
     ::exit(exitCode);
     break;
  }
  return false; // make insure++ happy
}


KUniqueApplication::KUniqueApplication(bool GUIenabled, bool configUnique)
  : KApplication( GUIenabled, initHack( configUnique )),
    DCOPObject(KCmdLineArgs::about->appName()),d(new Private)
{
  d->processingRequest = false;
  d->firstInstance = true;

  if (s_nofork)
    // Can't call newInstance directly from the constructor since it's virtual...
    QTimer::singleShot( 0, this, SLOT(newInstanceNoFork()) );
}


#ifdef Q_WS_X11
KUniqueApplication::KUniqueApplication(Display *display, Qt::HANDLE visual,
		Qt::HANDLE colormap, bool configUnique)
  : KApplication( display, visual, colormap, initHack( configUnique )),
    DCOPObject(KCmdLineArgs::about->appName()),d(new Private)
{
  d->processingRequest = false;
  d->firstInstance = true;

  if (s_nofork)
    // Can't call newInstance directly from the constructor since it's virtual...
    QTimer::singleShot( 0, this, SLOT(newInstanceNoFork()) );
}
#endif


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
    KConfigGroup cg( inst->config(), "KDE" );
    s_multipleInstances = cg.readBoolEntry("MultipleInstances", false);
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

  s_handleAutoStarted = false;
  newInstance();
  d->firstInstance = false;
#if defined Q_WS_X11
  // KDE4 remove
  // A hack to make startup notification stop for apps which override newInstance()
  // and reuse an already existing window there, but use KWin::activateWindow()
  // instead of KStartupInfo::setNewStartupId(). Therefore KWin::activateWindow()
  // for now sets this flag. Automatically ending startup notification always
  // would cause problem if the new window would show up with a small delay.
  if( s_handleAutoStarted )
      KStartupInfo::handleAutoAppStartedSending();
#endif
  // What to do with the return value ?
}

bool KUniqueApplication::process(const DCOPCString &fun, const QByteArray &data,
				 DCOPCString &replyType, QByteArray &replyData)
{
  if (fun == "newInstance()")
  {
    delayRequest(fun, data);
    return true;
  } else
    return DCOPObject::process(fun, data, replyType, replyData);
}

void
KUniqueApplication::delayRequest(const QByteArray &fun, const QByteArray &data)
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
     DCOPRequest *request = d->requestList.takeFirst();
     QByteArray replyData;
     DCOPCString replyType;
     if (request->fun == "newInstance()") {
       dcopClient()->setPriorityCall(false);
       QDataStream ds(&request->data, QIODevice::ReadOnly);
       ds.setVersion( QDataStream::Qt_3_1 );
       KCmdLineArgs::loadAppArgs(ds);
       if( !ds.atEnd()) // backwards compatibility
       {
           QByteArray asn_id;
           ds >> asn_id;
           setStartupId( asn_id );
       }
       s_handleAutoStarted = false;
       int exitCode = newInstance();
       d->firstInstance = false;
#if defined Q_WS_X11
       if( s_handleAutoStarted )
           KStartupInfo::handleAutoAppStartedSending(); // KDE4 remove?
#endif
       QDataStream rs(&replyData, QIODevice::WriteOnly);
       rs << exitCode;
       replyType = "int";
     }
     dcopClient()->endTransaction( request->transaction, replyType, replyData);
     delete request;
  }

  d->processingRequest = false;
}

bool KUniqueApplication::restoringSession()
{
  return d->firstInstance && isSessionRestored();
}

int KUniqueApplication::newInstance()
{
  if (!d->firstInstance)
  {

    if ( mainWidget() )
    {
      mainWidget()->show();
#if defined Q_WS_X11
    // This is the line that handles window activation if necessary,
    // and what's important, it does it properly. If you reimplement newInstance(),
    // and don't call the inherited one, use this (but NOT when newInstance()
    // is called for the first time, like here).
      KStartupInfo::setNewStartupId( mainWidget(), kapp->startupId());
#endif
    }
  }
  return 0; // do nothing in default implementation
}

void KUniqueApplication::setHandleAutoStarted()
{
    s_handleAutoStarted = false;
}

void KUniqueApplication::virtual_hook( int id, void* data )
{ KApplication::virtual_hook( id, data );
  DCOPObject::virtual_hook(id, data ); }

#include "kuniqueapplication.moc"
