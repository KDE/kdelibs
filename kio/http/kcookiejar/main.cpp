/*
    This file is part of KDE 

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

/*
 * To debug add "--nofork" to the commandline!
 */

#include "kcookieserver.h"
#include <kcmdlineargs.h>
#include <klocale.h>
#include <stdlib.h>
#include <stdio.h>

static const char *description = 
	I18N_NOOP("HTTP Cookie Daemon");

static const char *version = "1.0";


int main(int argc, char *argv[])
{
   KCmdLineArgs::init(argc, argv, "kcookiejar", description, version);
   if (!KCookieServer::start())
   {
      printf("KCookieJar already running.\n");
      exit(0);
   }
   KCookieServer server;
   return server.exec(); // keep running
}
