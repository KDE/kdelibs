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
       "addCookies(QString,QCString,long int)", params))
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

static QString findDOMCookies( QString url )
{
   QCString replyType;
   QByteArray params, reply;
   QDataStream stream(params, IO_WriteOnly);
   stream << url;
   if (!kapp->dcopClient()->call("kcookiejar", "kcookiejar",
       "findDOMCookies(QString)", params, replyType, reply))
   {
      printf("There was some error using DCOP!\n");
      return QString::null;
   }

   QDataStream stream2(reply, IO_ReadOnly);
   if(replyType != "QString")
   {
      printf("DCOP function findDOMCookies(...) return %s, expected %s\n",
              replyType.data(), "QString");
      return QString::null;
   }

   QString result;
   stream2 >> result;
   return result;
}


int main(int argc, char *argv[])
{
   QString arg1;
   QCString arg2;
   QString result;

   KApplication k(argc, argv, "kcookiejartest");
   kapp->dcopClient()->attach();
   kapp->dcopClient()->registerAs( kapp->name()) ;

   if (!kapp->dcopClient()->isApplicationRegistered("kcookiejar"))
   {
      QString error;
       if (KApplication::startServiceByDesktopName("kcookiejar", QStringList(), &error ))
       {
          // Error starting kcookiejar.
          printf("Error starting KCookiejar: %s\n", error.latin1());
       }
   }

   arg1 = "http://e2.swirve.com/browser.cgi";
   printf("URL: %s\n", arg1.latin1());
   arg2 = "Set-Cookie: EarthID=MJEHEWPOIA; path=/; expires=Fri, 31-Dec-2002 00:00:00 GMT\n";
   arg2 += "Set-Cookie: TESTNAME=Earth; path=/";
   addCookies(arg1, arg2);
   
   arg1 = "http://e2.swirve.com";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: TESTNAME=Earth; EarthID=MJEHEWPOIA" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

/*
   // Add cookies...
   arg1 = "http://w.y.z/";
   printf("Set-Cookie request for: %s\n", arg1.latin1());
   arg2 = "Set-Cookie: some_value=value1; Path=\"/\"; expires=Fri, 04-May-2002 01:00:00 GMT";
   addCookies(arg1, arg2);

   arg1 = "http://x.y.z/";
   printf("Set-Cookie request for: %s\n", arg1.latin1());
   arg2 = "Set-Cookie: some_value=value2; Path=\"/\"; expires=Fri, 04-May-2002 01:00:00 GMT";
   addCookies(arg1, arg2);

   arg1 = "http://foo.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: some_value=value3; expires=Fri, 04-May-2002 01:00:00 GMT";
   addCookies(arg1, arg2);

   arg1 = "http://www.foo.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: some_value=value4; expires=Fri, 04-May-2002 01:00:00 GMT";
   addCookies(arg1, arg2);

   arg1 = "http://www.acme.com";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: some_value=value5; expires=Fri, 04-May-2002 01:00:00 GMT";
   addCookies(arg1, arg2);

   arg1 = "http://acme.com";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: some_value=value6; expires=Fri, 04-May-2002 01:00:00 GMT";
   addCookies(arg1, arg2);



   // Find cookies...
   arg1 = "http://w.y.z/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("RESULT: %s\n\n", result.latin1() );

   arg1 = "http://x.y.z/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("RESULT: %s\n\n", result.latin1() );

   arg1 = "http://www.foo.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("RESULT: %s\n\n", result.latin1() );

   arg1 = "http://foo.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("RESULT: %s\n\n", result.latin1() );

   arg1 = "http://www.acme.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("RESULT: %s\n\n", result.latin1() );

   arg1 = "http://acme.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("RESULT: %s\n\n", result.latin1() );

   /********************************************************* Space in the value test
   arg1 = "http://x.y.z/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: another_value=\"abcd efgh ijkl mnopq rstuv wxyz\"; Path=\"/\"; domain=\".y.z\"";
   addCookies(arg1, arg2);

   arg1 = "http://x.y.z/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("RESULT: %s\n\n", result.latin1() );


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
   arg2 = ("Set-Cookie: set_by_6=\"f22.w.x.y.foobar.com\"; path=\"/\"; domain=\".foobar.com\"\n"
           "Set-Cookie: set_by_7=\"f22.w.x.y.foobar.com\"; path=\"/acme/test\"; domain=\".foobar.com\"");
   addCookies(arg1, arg2);

   // Domain should be corrected to ".y.foobar.co.uk"
   arg1 = "http://x.y.foobar.co.uk/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_8=\"x.y.foobar.co.uk\"; Path=\"/\"; Domain=\".foobar.co.uk\"";
   addCookies(arg1, arg2);

   // Testing a missing "." in the specified domain name. This
   // should result in the cookie being set for ".foo.com".
   arg1 = "http://y.foo.com/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_9=\"y.foo.com\"; Domain=\"foo.com\"";
   addCookies(arg1, arg2);

   // A little twist in the previous test. Increased the domain level
   // by one to see if the same test would pass...  This should result
   // in a cookie being set for ".y.foo.com".
   arg1 = "http://x.y.foo.com/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_A=\"x.y.foo.com\"; Domain=\"foo.com\"";
   addCookies(arg1, arg2);

   // A miscevious test.  This should appear in ".foobar.com".  In
   // the old code it appeared in "foobar.com" and caused problems
   // if one then went to www.foobar.com basically the same site!!
   // Real world examples: linuxtoday.com, slashdot.org, linux.com...
   arg1 = "http://foobar.com/acme/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_B=\"foobar.com\"";
   addCookies(arg1, arg2);

   // Cookie that should be sent iff the request is secure...
   arg1 = "https://www.secure-foo.com/acme/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_C=\"www.secure-foo.com\"; Path=\"/\"; Secure";
   addCookies(arg1, arg2);

   // Cookie that should be sent iff the request is secure...
   arg1 = "https://foo.secure-store.com/acme/index.html";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_D=\"foo.secure-store.com\"; Path=\"/\"; expires=Sat, 30 Sep 2000 00:00:00 GMT; Secure";
   addCookies(arg1, arg2);


   // Anything below here should simply be flat-out rejected by the cookiejar!!
   // The user should not even see these at all!
   arg1 = "http://www.foobar.co.uk/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_AA=\"www.foobar.co.uk\"; Path=\"/\"; Domain=\".foorbar.com\"";
   addCookies(arg1, arg2);

   arg1 = "http://www.foo-foobar.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_BB=\"www.foo-foobar.com\"; Path=\"/\"; Domain=\".com\"";
   addCookies(arg1, arg2);

   arg1 = "http://www.foobar.com/";
   printf("Requesting to set cookie for %s\n", arg1.latin1());
   arg2 = "Set-Cookie: set_by_CC=\"www.foobar.com\"; Path=\"/\"; Domain=\".foobar.co.uk\"";
   addCookies(arg1, arg2);



   // Should PASS.
   arg1 = "http://www.foo.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_9=y.foo.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://www.foo.com/acme";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_9=y.foo.com set_by_0=www.foo.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://x.y.foo.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie set_by_A=x.y.foo.com set_by_9=y.foo.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://www.foobar.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_1=www.foobar.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://www.foobar2.com/acme";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_2=www.foobar2.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://y.foobar.com/acme/index.html";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_1=www.foobar.com; set_by_B=foobar.com; set_by_3=y.foobar.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://x.y.foobar.com/acme/index.html";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_4=x.y.foobar.com; set_by_1=www.foobar.com; set_by_B=foobar.com; set_by_3=y.foobar.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://w.x.y.foobar.com/acme/index.html";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_5=w.x.y.foobar.com; set_by_4=x.y.foobar.com; set_by_1=www.foobar.com; set_by_B=foobar.com; set_by_3=y.foobar.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://f22.w.x.y.foobar.com/config/login?7645kd30948jdha";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_6=f22.w.x.y.foobar.com; set_by_7=f22.w.x.y.foobar.com; set_by_1=www.foobar.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS
   arg1 = "https://www.secure-foo.com/acme/index.html";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "Cookie: set_by_C=www.secure-foo.com" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should FAIL.  We should not send secure cookies through insecure links...
   arg1 = "http://www.secure-foo.com/acme/index.html";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "<NULL>" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://foobar.co.uk/acme/index.html";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "set_by_8=x.y.foobar.co.uk" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://www.foobar.co.uk/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "set_by_8=x.y.foobar.co.uk" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should FAIL.
   arg1 = "http://www.foo-foobar.com/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "<NULL>" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should FAIL. Spoofing test!!
   arg1 = "http://www.peacefire.org%2fsecurity%2fiecookies%2fshowcookie.html%3F.foobar.com";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findCookies(arg1);
   printf("EXPECTED: %s\n", "<NULL>" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");

   // Should PASS.
   arg1 = "http://www.foobar.co.uk/";
   printf("Looking up cookies for %s \n", arg1.latin1());
   result = findDOMCookies(arg1);
   printf("EXPECTED: %s\n", "set_by_8=x.y.foobar.co.uk" );
   printf("RESULT: %s\n\n", result.latin1() ? result.latin1() : "<NULL>");
*/
}
