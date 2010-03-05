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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <unistd.h>
#include <fcntl.h>

#include "klauncher.h"
#include <kcomponentdata.h>
#include "kcrash.h"
#include "kdebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <klocale.h>
#include <kde_file.h>

#include "klauncher_cmds.h"
#include <QtCore/QCoreApplication>

#ifndef Q_WS_WIN
static int sigpipe[ 2 ];
static void sig_handler(int sig_num)
{
   // No recursion
   KDE_signal( SIGHUP, SIG_IGN);
   KDE_signal( SIGTERM, SIG_IGN);
   fprintf(stderr, "klauncher: Exiting on signal %d\n", sig_num);
   char tmp = 'x';
   write( sigpipe[ 1 ], &tmp, 1 );
}
#endif

#if defined(Q_OS_DARWIN) || defined (Q_OS_MAC)
#include <kkernel_mac.h>
#endif

extern "C" KDE_EXPORT int kdemain( int argc, char**argv )
{
#ifndef Q_WS_WIN
   // Started via kdeinit.
   int launcherFd;
   if (argc != 2 || memcmp(argv[1], "--fd=", 5) || !(launcherFd = atoi(argv[1] + 5)))
   {
      fprintf(stderr, "%s", i18n("klauncher: This program is not supposed to be started manually.\n"
                                 "klauncher: It is started automatically by kdeinit4.\n").toLocal8Bit().data());
      return 1;
   }
#endif

#if defined(Q_OS_DARWIN) || defined (Q_OS_MAC)
   mac_initialize_dbus();
#endif

   KComponentData componentData("klauncher", "kdelibs4");
   KGlobal::locale();

   // WABA: Make sure not to enable session management.
   putenv(strdup("SESSION_MANAGER="));

   // We need a QCoreApplication to get a DBus event loop
   QCoreApplication app(argc, argv);
   app.setApplicationName( componentData.componentName() );

   int maxTry = 3;
   while(true)
   {
      QString service(QLatin1String("org.kde.klauncher")); // same as ktoolinvocation.cpp
      if (!QDBusConnection::sessionBus().isConnected()) {
         kWarning() << "No DBUS session-bus found. Check if you have started the DBUS server.";
         return 1;
      }
      QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply =
          QDBusConnection::sessionBus().interface()->registerService(service);
      if (!reply.isValid())
      {
         kWarning() << "DBUS communication problem!";
         return 1;
      }
      if (reply == QDBusConnectionInterface::ServiceRegistered)
          break;

      if (--maxTry == 0)
      {
         kWarning() << "Another instance of klauncher is already running!";
         return 1;
      }

      // Wait a bit...
      kWarning() << "Waiting for already running klauncher to exit.";
      sleep(1);

      // Try again...
   }

#ifndef Q_WS_WIN
   KLauncher *launcher = new KLauncher(launcherFd);
#else
   KLauncher *launcher = new KLauncher();
#endif
   QDBusConnection::sessionBus().registerObject(QString::fromLatin1("/"), launcher);

#ifndef Q_WS_WIN
   if (pipe(sigpipe) != 0) {
       perror("klauncher: pipe failed.");
       return 1;
   }
   QSocketNotifier* signotif = new QSocketNotifier( sigpipe[ 0 ], QSocketNotifier::Read, launcher );
   QObject::connect( signotif, SIGNAL( activated( int )), launcher, SLOT( destruct()));
   KCrash::setEmergencySaveFunction(sig_handler);
   KDE_signal( SIGHUP, sig_handler);
   KDE_signal( SIGPIPE, SIG_IGN);
   KDE_signal( SIGTERM, sig_handler);
#endif

   return app.exec();
}
