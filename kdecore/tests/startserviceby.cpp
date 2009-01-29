/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>
    Copyright (c) 2009 David Faure <faure@kde.org>

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

#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <ktoolinvocation.h>
#include <kdebug.h>

int
main(int argc, char *argv[])
{
   KAboutData about("ktoolinvocationtest", 0, ki18n("ktoolinvocationtest"), "version");
   KComponentData cData(&about);
   //KCmdLineArgs::init(argc, argv, &about);
   //KApplication a;

   QString serviceId = "kwrite.desktop";
   if (argc > 1) {
       serviceId = QString::fromLocal8Bit(argv[1]);
   }
   QString url;
   if (argc > 2) {
       url = QString::fromLocal8Bit(argv[2]);
   }

   QString error;
   QString dbusService;
   int pid;
   KToolInvocation::startServiceByDesktopPath( serviceId, url, &error, &dbusService, &pid );
   kDebug() << "Started. error=" << error << " dbusService=" << dbusService << " pid=" << pid;

   return 0;
}
