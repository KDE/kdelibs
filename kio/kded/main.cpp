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
#include <kinstance.h>
#include <dcopclient.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static const char *appName = "kbuildsycoca";

int main(int, char **)
{
   KInstance k("kbuildsycoca");

   DCOPClient *dcopClient = new DCOPClient();

   if (dcopClient->registerAs(appName) != appName)
   {
     fprintf(stderr, "%s already running!\n", appName);
     exit(0);
   }

   KBuildSycoca *sycoca= new KBuildSycoca; // Build data base
   sycoca->recreate();
}

