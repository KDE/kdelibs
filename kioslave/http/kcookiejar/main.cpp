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

#define NOFORK

#include "kcookieserver.h"
#include <kuniqueapp.h>
#include <dcopclient.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   KUniqueApplication k(argc, argv, "kcookiejar", true /* has a GUI */);
 
   kapp->dcopClient()->attach();
   kapp->dcopClient()->registerAs( kapp->name()) ;

   KCookieServer server;

#ifndef NOFORK     // define NOFORK to debug kcookiejar in gdb
     QApplication::flushX();	
     switch(fork()) {
     case -1:
	  fprintf(stderr, "kcookiejar: fork() failed!\n");
	  break;
     case 0:
	  // ignore in child
	  break;
     default:
	  // parent: exit immediatly
          fprintf(stdout, "kcookiejar now running in the background\n");
	  _exit(0);
     }
#endif

     return k.exec(); // keep running
}
