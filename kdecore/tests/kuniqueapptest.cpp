/* This file is part of the KDE libraries
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

#include "kuniqueapplication.h"
#include "kglobalsettings.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

class TestApp : public KUniqueApplication
{
public:
   TestApp() : KUniqueApplication("TestApp") { }
   virtual int newInstance( );
};


int
TestApp::newInstance( )
{
   qWarning("NewInstance");
   return 0;
}

int
main(int argc, char *argv[])
{
   KAboutData about("kuniqueapptest", "kuniqueapptest", "version");
   KCmdLineArgs::init(argc, argv, &about);
   KUniqueApplication::addCmdLineOptions();

   if (!TestApp::start())
   {
      exit(0);
   }
   TestApp a;

   printf("Running.\n");
   kapp->exec();
   printf("Terminating.\n");
}
