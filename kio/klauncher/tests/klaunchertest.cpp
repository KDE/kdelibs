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

#include <qstring.h>
#include <kapp.h>
#include <dcopclient.h>
#include <stdio.h>
#include <qvaluelist.h>
#include <kservice.h>

/*
static void
exec_blind( QCString name, const QValueList<QCString> &args)
{
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << name << args;
   if (!kapp->dcopClient()->send("klauncher", "klauncher", 
	"exec_blind(QCString, QValueList<QCString>)", params))
	printf("There was some error using DCOP!\n");
}
*/

int main(int argc, char *argv[])
{
   KApplication k(argc, argv, "klaunchertest");
 
   kapp->dcopClient()->registerAs( kapp->name()) ;

   QValueList<QCString> args;

//   exec_blind("konsole", args);


   QString error;
   QCString dcopService;
   int pid;
   int result = KApplication::startServiceByDesktopName(
		QString::fromLatin1("konsole"), QString::null, &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);

   result = KApplication::startServiceByDesktopName(
		QString::fromLatin1("konqueror"), QString::null,  &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);
}

