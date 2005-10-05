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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <qstring.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <stdio.h>
#include <kservice.h>
#include <ktoolinvocation.h>
#include <kcmdlineargs.h>
#include <ktoolinvocation.h>

int main(int argc, char *argv[])
{
   KToolInvocation::kdeinitExec("konsole");
   KCmdLineArgs::init( argc, argv, "klaunchertest", 0, 0, 0, 0);
   KApplication k;

   KApplication::dcopClient()->registerAs( kapp->name()) ;

#if 0
   QString error;
   QByteArray dcopService;
   int pid;
   int result = KApplication::startServiceByDesktopName(
		QLatin1String("konsole"), QString::null, &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);

   result = KApplication::startServiceByDesktopName(
		QLatin1String("konqueror"), QString::null,  &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);
#endif
}

