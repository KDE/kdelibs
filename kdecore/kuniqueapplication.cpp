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
#include <dbus/qdbus.h>

#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

#if defined Q_WS_X11
#include <kwin.h>
#include <kstartupinfo.h>
#endif

/* I don't know why, but I end up with complaints about
   a forward-declaration of QWidget in the activeWidow()->show
   call below on Qt/Mac if I don't include this here... */
#include <QWidget>

#include <kconfig.h>
#include "kdebug.h"
#include "kuniqueapplication.h"
#include "kuniqueapplication_p.h"

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
bool s_kuniqueapplication_startCalled = false;
bool KUniqueApplication::s_handleAutoStarted = false;

static KCmdLineOptions kunique_options[] =
{
  { "nofork", "Don't run in the background.", 0 },
  KCmdLineLastOption
};

class KUniqueApplication::Private
{
public:
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
  if( s_kuniqueapplication_startCalled )
    return true;
  s_kuniqueapplication_startCalled = true;
  addCmdLineOptions(); // Make sure to add cmd line options
#ifdef Q_WS_WIN
  s_nofork = true;
#else
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kuniqueapp");
  s_nofork = !args->isSet("fork");
  delete args;
#endif

  // Check the D-Bus connection health
  QDBusBusService* dbusService = 0;
  if (!QDBus::sessionBus().isConnected() || !(dbusService = QDBus::sessionBus().busService()))
  {
    kError() << "KUniqueApplication: Cannot find the D-Bus session server" << endl;
    ::exit(255);
  }

  QString appName = QString::fromLatin1(KCmdLineArgs::about->appName());
  const QStringList parts = KCmdLineArgs::about->organizationDomain().split(QLatin1Char('.'), QString::SkipEmptyParts);
  if (parts.isEmpty())
     appName.prepend(QLatin1String("local."));
  else
     foreach (const QString& s, parts)
     {
        appName.prepend(QLatin1Char('.'));
        appName.prepend(s);
     }

  if (s_nofork)
  {
     if (s_multipleInstances)
     {
        QString pid = QString::number(getpid());
        appName = appName + '-' + pid;
     }

     // Check to make sure that we're actually able to register with the D-Bus session
     // server.

#ifndef Q_WS_WIN //TODO
     if (dbusService->requestName(appName, QDBusBusService::DoNotQueueName) != QDBusBusService::PrimaryOwnerReply)
     {
        kError() << "KUniqueApplication: Can't setup D-Bus service. Probably already running."
                 << endl;
        ::exit(255);
     }
#endif

     // We'll call newInstance in the constructor. Do nothing here.
     return true;
  }

  int fd[2];
  signed char result;
  if (0 > pipe(fd))
  {
     kError() << "KUniqueApplication: pipe() failed!" << endl;
     ::exit(255);
  }
  int fork_result = fork();
  switch(fork_result) {
  case -1:
     kError() << "KUniqueApplication: fork() failed!" << endl;
     ::exit(255);
     break;
  case 0:
     {
        // Child
        ::close(fd[0]);
        if (s_multipleInstances)
           appName.append("-").append(QString::number(getpid()));

        QDBusReply<QDBusBusService::RequestNameReply> reply =
            dbusService->requestName(appName, QDBusBusService::DoNotQueueName);
        if (reply.isError())
        {
           kError() << "KUniqueApplication: Can't setup D-Bus service." << endl;
           result = -1;
           ::write(fd[1], &result, 1);
           ::exit(255);
        }
        if (reply == QDBusBusService::NameExistsReply)
        {
           // Already running. Ok.
           result = 0;
           ::write(fd[1], &result, 1);
           ::close(fd[1]);
           return false;
        }

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
     if (s_multipleInstances)
        appName.append("-").append(QString::number(fork_result));
     ::close(fd[1]);

     // Create a secondary connection to the D-BUS server
     // The primary one (QDBus::sessionBus()) belongs to the child
     QDBusConnection con = QDBusConnection::addConnection(QDBusConnection::SessionBus, "kuniqueapplication");
     dbusService = 0;
     if (!con.isConnected() || !(dbusService = con.busService()))
     {
       kError() << "KUniqueApplication: Cannot create secondary connection to the D-BUS server" << endl;
       ::exit(255);
     }
     forever
     {
       int n = ::read(fd[0], &result, 1);
       if (n == 1) break;
       if (n == 0)
       {
          kError() << "KUniqueApplication: Pipe closed unexpectedly." << endl;
          ::exit(255);
       }
       if (errno != EINTR)
       {
          kError() << "KUniqueApplication: Error reading from pipe." << endl;
          ::exit(255);
       }
     }
     ::close(fd[0]);

     if (result != 0)
        ::exit(result); // Error occurred in child.

     if (!dbusService->nameHasOwner(appName))
     {
        kError() << "KUniqueApplication: Registering failed!" << endl;
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

     QDBusInterface *iface = con.findInterface(appName, "/MainApplication",
                                               "org.kde.KUniqueApplication");
     QDBusReply<int> reply;
     if (!iface || (reply = iface->call("newInstance", new_asn_id)).isError())
     {
       QDBusError err = iface->lastError();
        kError() << "Communication problem with " << KCmdLineArgs::about->appName() << ", it probably crashed." << endl
                 << "Error message was: " << err.name() << ": \"" << err.message() << "\"" << endl;
        ::exit(255);
     }
     delete iface;
     ::exit(reply);
     break;
  }
  return false; // make insure++ happy
}


KUniqueApplication::KUniqueApplication(bool GUIenabled, bool configUnique)
  : KApplication( GUIenabled, initHack( configUnique )),
    d(new Private)
{
  d->processingRequest = false;
  d->firstInstance = true;

  // the sanity checking happened in initHack
  new KUniqueApplicationAdaptor(this);

  if (s_nofork)
    // Can't call newInstance directly from the constructor since it's virtual...
    QTimer::singleShot( 0, this, SLOT(newInstanceNoFork()) );
}


#ifdef Q_WS_X11
KUniqueApplication::KUniqueApplication(Display *display, Qt::HANDLE visual,
		Qt::HANDLE colormap, bool configUnique)
  : KApplication( display, visual, colormap, initHack( configUnique )),
    d(new Private)
{
  d->processingRequest = false;
  d->firstInstance = true;

  // the sanity checking happened in initHack
  new KUniqueApplicationAdaptor(this);

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
     s_multipleInstances = cg.readEntry("MultipleInstances", false);
  }
  if( !start())
     // Already running
     ::exit( 0 );
  return inst;
}

void KUniqueApplication::newInstanceNoFork()
{
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

bool KUniqueApplication::restoringSession()
{
  return d->firstInstance && isSessionRestored();
}

int KUniqueApplication::newInstance()
{
  if (!d->firstInstance)
  {

    if ( activeWindow() )
    {
      activeWindow()->show();
#if defined Q_WS_X11
    // This is the line that handles window activation if necessary,
    // and what's important, it does it properly. If you reimplement newInstance(),
    // and don't call the inherited one, use this (but NOT when newInstance()
    // is called for the first time, like here).
      KStartupInfo::setNewStartupId( activeWindow(), kapp->startupId());
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
{ KApplication::virtual_hook( id, data ); }

#include "kuniqueapplication.moc"
#include "kuniqueapplication_p.moc"
