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
#include <qcstring.h>
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

extern "C" int kdemain( int argc, char**argv )
{
   // Started via kdeinit.
   if (fcntl(LAUNCHER_FD, F_GETFD) == -1)
   {
      fprintf(stderr, "%s", i18n("klauncher: This program is not supposed to be started manually.\n"
                                 "klauncher: It is started automatically by kdeinit.\n").local8Bit().data());
      return 1;
   }

   QCString cname = KApplication::launcher();
   char *name = cname.data();
   KCmdLineArgs::init(argc, argv, name, "KLauncher", "A service launcher.",
                       "v1.0");

   KLauncher::addCmdLineOptions();

   // WABA: Make sure not to enable session management.
   putenv(strdup("SESSION_MANAGER="));

   // Allow the locale to initialize properly
   KLocale::setMainCatalogue("kdelibs");

   // Check DCOP communication.
   {
      DCOPClient testDCOP;
      QCString dcopName = testDCOP.registerAs(cname, false);
      if (dcopName.isEmpty())
      {
         kdWarning() << "DCOP communication problem!" << endl;
         return 1;
      }
      if (dcopName != cname)
      {
         kdWarning() << "Already running!" << endl;
      }
   }

   KApplication::dcopClient()->registerAs(name, false);
   KLauncher *launcher = new KLauncher(LAUNCHER_FD);
   launcher->dcopClient()->setDefaultObject( name );
   launcher->dcopClient()->setDaemonMode( true );

   KCrash::setEmergencySaveFunction(sig_handler);
   signal( SIGHUP, sig_handler);
   signal( SIGPIPE, SIG_IGN);
   signal( SIGTERM, sig_handler);

   launcher->exec();
   return 0;
}

