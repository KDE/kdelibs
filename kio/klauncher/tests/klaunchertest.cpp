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


struct serviceResult 
{
   int result;
   QCString dcopName;
   QString error;
};

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

static int
start_service( const QString &name, const QString &filename, 
               QCString &dcopService, QString &error)
{
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << name << filename;
   QCString replyType;
   QByteArray replyData;
   if (!kapp->dcopClient()->call("klauncher", "klauncher", 
	"start_service(QString,QString)", params, replyType, replyData))
   {
	printf("There was some error using DCOP!\n");
        return -1;
   }

   QDataStream stream2(replyData, IO_ReadOnly);
   serviceResult result;
   stream2 >> result.result >> result.dcopName >> result.error;
   dcopService = result.dcopName;
   error = result.error;
   return result.result;
}

int main(int argc, char *argv[])
{
   KApplication k(argc, argv, "klaunchertest");
 
   kapp->dcopClient()->registerAs( kapp->name()) ;

   QValueList<QCString> args;

   exec_blind("konsole", args);

   char buffer[1023];
   gets(buffer);
  
   QString error;
   QCString dcopService;
   int result = start_service("Terminal", QString::null, dcopService, error);

   printf("Result = %d, error = \"%s\", dcopService = \"%s\"\n",
      result, dcopService.data(), error.ascii());
}
