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

static void
addCookies( QString url, QCString cookieHeader)
{
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   stream << url << cookieHeader;
   if (!kapp->dcopClient()->send("kcookiejar", "kcookiejar", 
	"addCookies(QString, QCString)", params))
	printf("There was some error using DCOP!\n");
}

static QString
findCookies( QString url)
{
   QCString replyType;
   QByteArray params, reply;
   QDataStream stream(params, IO_WriteOnly);
   stream << url;
   if (!kapp->dcopClient()->call("kcookiejar", "kcookiejar", 
	"findCookies(QString)", params, replyType, reply))
   {
	printf("There was some error using DCOP!\n");
        return QString::null;
   }

   QDataStream stream2(reply, IO_ReadOnly);
   if(replyType != "QString")
   {
      printf("DCOP function findCookies(...) return %s, expected %s\n",
		replyType.data(), "QString");
      return QString::null;
   }

   QString result;
   stream2 >> result;

   return result;
}


int main(int argc, char *argv[])
{
   KApplication k(argc, argv, "kcookiejartest");
 
   kapp->dcopClient()->attach();
   kapp->dcopClient()->registerAs( kapp->name()) ;

   QString arg1;
   QCString arg2;
   QString result;

   arg1 = "http://www.somehost.com/";
   arg2 = "Set-Cookie: Part_Number=\"Rocket_Launcher_0001\"; Version=\"1\"; Path=\"/acme\"";
   addCookies(arg1, arg2);

   arg1 = "http://www.otherhost.com/";
   arg2 = "Set-Cookie: Part_Number=\"Rocket_Launcher_0001\"; Version=\"1\"; Path=\"/ocme\"";
   addCookies(arg1, arg2);

   arg1 = "http://www.somehost.com/";
   arg2 = "Set-Cookie: Part_Number=\"Rocket_Launcher_0002\"; Version=\"1\"; Path=\"/ocme\"";
   addCookies(arg1, arg2);

   arg1 = "http://www.somehost.com/acme/index.html";
   printf("Looking up cookies for %s\n", arg1.ascii());
   result = findCookies(arg1);
   printf("Result: %s\n", result.ascii() ? result.ascii() : "<NULL>");
}
