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

static void addCookies( QString url, QCString cookieHeader)
{
   QByteArray params;
   QDataStream stream(params, IO_WriteOnly);
   long int winId;
   stream << url << cookieHeader << winId;
   if (!kapp->dcopClient()->send("kcookiejar", "kcookiejar", 
	"addCookies(QString, QCString,long)", params))
	printf("There was some error using DCOP!\n");
}

static QString findCookies( QString url)
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

   // Simple Case tests...
   arg1 = "http://www.foo.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_0=\"www.foo.com\"; Path=\"/acme\"";
   addCookies(arg1, arg2);

   // Another simple test
   arg1 = "http://www.foobar.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_1=\"www.foobar.com\";";
   addCookies(arg1, arg2);

   // Yet another one
   arg1 = "http://www.foobar2.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_2=\"www.foobar2.com\"; Path=\"/acme\"";
   addCookies(arg1, arg2);

   // Should be saved correctly
   arg1 = "http://y.foobar.com/acme/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_3=\"y.foobar.com\"; Path=\"/acme\"; Domain=\".foobar.com\"";
   addCookies(arg1, arg2);

   // Domain should be corrected to ".y.foobar.com"
   arg1 = "http://x.y.foobar.com/acme/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_4=\"x.y.foobar.com\"; Path=\"/acme\"; Domain=\".foobar.com\"";
   addCookies(arg1, arg2);

   // Domain should be corrected to ".x.y.foobar.com"
   arg1 = "http://w.x.y.foobar.com/acme/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_5=\"w.x.y.foobar.com\"; Path=\"/acme\"; Domain=\".y.foobar.com\"";
   addCookies(arg1, arg2);

   // Domain should be corrected to "w.x.y.foobar.com"
   arg1 = "http://f22.w.x.y.foobar.com/config/login?8al4htk9mn3l9";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = ("Set-Cookie: set_by_7=\"f22.w.x.y.foobar.com\"; path=\"/\"; domain=\".foobar.com\"\n"
           "Set-Cookie: set_by_8=\"f22.w.x.y.foobar.com\"; path=\"/\"; domain=\".foobar.com\"");
   addCookies(arg1, arg2);

   // Domain should be corrected to ".y.foobar.co.uk"
   arg1 = "http://x.y.foobar.co.uk/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_A=\"x.y.foobar.co.uk\"; Path=\"/\"; Domain=\".foobar.co.uk\"";
   addCookies(arg1, arg2);

   // A miscevious test.  This should appear in ".foobar.com".  In
   // the old code it appeared in "foobar.com" and caused problems
   // if one then went to www.foobar.com basically the same site!!
   // Real world examples: linuxtoday.com, slashdot.org, linux.com...
   arg1 = "http://foobar.com/acme/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_6=\"foobar.com\"";
   addCookies(arg1, arg2);

   /* Anything below here should simply be flat-out rejected by the cookiejar!!
      The user should not even see these at all! */
   arg1 = "http://www.foobar.co.uk/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_9=\"www.foobar2.com\"; Path=\"/\"; Domain=\".foorbar.com\"";
   addCookies(arg1, arg2);

   arg1 = "http://www.foo-foobar.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_B=\"www.foo-foobar.com\"; Path=\"/\"; Domain=\".com\"";
   addCookies(arg1, arg2);

   arg1 = "http://www.foobar.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_B=\"www.foo-foobar.com\"; Path=\"/\"; Domain=\".foobar.co.uk\"";
   addCookies(arg1, arg2);

/* ****************************************************************************************************************** */

   // Should FAIL. Return <NULL>
   arg1 = "http://www.foo.com/";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS. Returning "Cookie: set_by_0=www.foo.com"
   arg1 = "http://www.foo.com/acme";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS. Returning "Cookie: set_by_1=www.foobar.com"
   arg1 = "http://www.foobar.com/";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS. Returning "Cookie: set_by_2=www.foobar2.com"
   arg1 = "http://www.foobar2.com/acme";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS. Returning "Cookie: set_by_1=www.foobar.com; set_by_6=foobar.com; set_by_3=y.foobar.com"
   arg1 = "http://y.foobar.com/acme/index.html";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS. Returning "Cookie: set_by_4=x.y.foobar.com; set_by_1=www.foobar.com; set_by_6=foobar.com; set_by_3=y.foobar.com"
   arg1 = "http://x.y.foobar.com/acme/index.html";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS. Returning "Cookie: set_by_5=w.x.y.foobar.com; set_by_4=x.y.foobar.com; set_by_1=www.foobar.com; set_by_6=foobar.com; set_by_3=y.foobar.com"
   arg1 = "http://w.x.y.foobar.com/acme/index.html";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS. Returning "Cookie: set_by=f22.w.x.y.foobar.com; set_by=f22.w.x.y.foobar.com; set_by_1=www.foobar.com"
   arg1 = "http://f22.w.x.y.foobar.com/config/login?7645kd30948jdha";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should FAIL. Return <NULL>
   arg1 = "http://foobar.co.uk/acme/index.html";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should FAIL. Return <NULL>
   arg1 = "http://www.foobar.co.uk/";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should FAIL. Return <NULL>
   arg1 = "http://www.foo-foobar.com/";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should FAIL. Return <NULL>. Important spoofing test to see if we pass...
   arg1 = "http://www.peacefire.org%2fsecurity%2fiecookies%2fshowcookie.html%3F.foobar.com";
   printf("===== Looking up cookies for %s =====\n", arg1.latin1());
   result = findCookies(arg1);
   printf("Result: %s\n", result.latin1() ? result.latin1() : "<NULL>");

}
