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

#include "kuniqueapp.h"
#include "kglobal.h"

#include <unistd.h>
#include <stdio.h>

class TestApp : public KUniqueApplication
{
public:
   TestApp(int argc, char *argv[])
	: KUniqueApplication(argc, argv, "TestApp") { }
   int newInstance( QValueList<QCString> params );
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
   if (!TestApp::start(argc,argv, "TestApp"))
   {
//      printf("Already running!\n");
      exit(0);
   }
   TestApp a(argc, argv);

   printf("Running.\n");
   printf("useDoubleClicks? %s\n", KGlobal::useDoubleClicks() ? "yes" : "no");
   printf("completionMode: %d\n", (int) KGlobal::completionMode());
   kapp->exec();   
   printf("Terminating.\n");
}
