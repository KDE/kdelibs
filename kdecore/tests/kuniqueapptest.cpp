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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
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
   virtual int newInstance( QValueList<QCString> params );
};


int
TestApp::newInstance( QValueList<QCString> params )
{
   printf("%s PID = %d params = %d\n", name(), getpid(), params.count());
   QValueList<QCString>::Iterator it = params.begin(); 
   int i = 0;
   for(;it != params.end(); it++, i++)
   {
      printf("arg %d: %s\n", i, (*it).data());
   }
   if ((params.count() != 2) || (params[1] == "--help"))
   {
       printf("Usage: %s [ --help | ok | exit ]\n", params[0].data());
       return -1;
   }
   if (params[1] == "ok")
   {
       printf("OK\n");
       return 1;
   }
   if (params[1] == "exit")
   {
       exit(0);
       printf("Exiting\n");
       return 1;
   }
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
//      printf("Already running!\n");
      exit(0);
   }
   TestApp a;

   printf("Running.\n");
   printf("singleClick? %s\n", KGlobalSettings::singleClick() ? "yes" : "no");
   printf("completionMode: %d\n", (int) KGlobalSettings::completionMode());
   kapp->exec();   
   printf("Terminating.\n");
}
