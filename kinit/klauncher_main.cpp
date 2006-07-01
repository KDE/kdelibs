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

#include "config.h"

#include <unistd.h>
#include <fcntl.h>

#include "kapplication.h"
#include "klauncher.h"
#include "kcmdlineargs.h"
#include "kcrash.h"
#include "kdebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <klocale.h>

#include "klauncher_cmds.h"

static void sig_handler(int sig_num)
{
   // No recursion
   signal( SIGHUP, SIG_IGN);
   signal( SIGTERM, SIG_IGN);
fprintf(stderr, "klauncher: Exiting on signal %d\n", sig_num);
   KLauncher::destruct(255);
}

extern "C" KDE_EXPORT int kdemain( int argc, char**argv )
{
   // Started via kdeinit.
   if (fcntl(LAUNCHER_FD, F_GETFD) == -1)
   {
      fprintf(stderr, "%s", i18n("klauncher: This program is not supposed to be started manually.\n"
                                 "klauncher: It is started automatically by kdeinit.\n").toLocal8Bit().data());
      return 1;
   }

   KCmdLineArgs::init(argc, argv, "klauncher", "KLauncher", "A service launcher.",
                       "v1.0");

   //KLauncher::addCmdLineOptions(); already done by kcmdlineargs

   // WABA: Make sure not to enable session management.
   putenv(strdup("SESSION_MANAGER="));

   // Allow the locale to initialize properly
   KLocale::setMainCatalog("kdelibs");

   int maxTry = 3;
   while(true)
   {
      QString service(QLatin1String("org.kde.klauncher")); // same as ktoolinvocation.cpp
      if (!QDBus::sessionBus().isConnected()) {
         kWarning() << "No DBUS session-bus found. Check if you have started the DBUS server." << endl;
         return 1;
      } 
      QDBusReply<QDBusConnectionInterface::RegisterServiceReply> reply =
          QDBus::sessionBus().interface()->registerService(service);
      if (!reply.isValid())
      {
         kWarning() << "DBUS communication problem!" << endl;
         return 1;
      }
      if (reply == QDBusConnectionInterface::ServiceRegistered)
          break;

      if (--maxTry == 0)
      {
         kWarning() << "Another instance of klauncher is already running!" << endl;
         return 1;
      }
      
      // Wait a bit...
      kWarning() << "Waiting for already running klauncher to exit." << endl;
      sleep(1);

      // Try again...
   }
   
   KLauncher *launcher = new KLauncher(LAUNCHER_FD);
   QDBus::sessionBus().registerObject("/", launcher);

   KCrash::setEmergencySaveFunction(sig_handler);
   signal( SIGHUP, sig_handler);
   signal( SIGPIPE, SIG_IGN);
   signal( SIGTERM, sig_handler);

   launcher->exec();
   return 0;
}
