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

#include "klauncher.h"
#include "kcmdlineargs.h"
#include <stdio.h>


extern "C" { int start_launcher(int); }

int
start_launcher(int socket)
{
   const char *name = "klauncher";
   // Started via kinit.
   KCmdLineArgs::init(1, (char **) &name, name, "A service launcher.", 
                       "v0.1");
   KLauncher::addCmdLineOptions();

   KLauncher launcher(socket);
   launcher.exec();
   return 0;
}

int main(/*int argc, char **argv*/)
{
   return 0;
}
