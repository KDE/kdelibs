/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
 
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kbuildsycoca.h"
#include <kapp.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
     KApplication k(argc,argv, "kded", false /* not GUI */);

     KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
     sycoca->recreate();

#ifndef NOFORK     // define NOFORK to debug kded in gdb
     switch(fork()) {
     case -1:
	  fprintf(stderr, "kded: fork() failed!\n");
	  break;
     case 0:
	  // ignore in child
	  break;
     default:
	  // parent: exit immediatly
          fprintf(stdout, "kded now running in the background\n");
	  _exit(0);
     }
#endif

     return k.exec(); // keep running
}
