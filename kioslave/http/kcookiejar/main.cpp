/*
This file is part of KDE

  Copyright (C) 1998-2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * To debug add "--nofork" to the commandline!
 */

#include <dcopclient.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapplication.h>

static const char *description =
	I18N_NOOP("HTTP Cookie Daemon");

static const char *version = "1.0";

static const KCmdLineOptions options[] =
{
 { "shutdown", I18N_NOOP("Shut down cookie jar."), 0 },
 { "remove <domain>", I18N_NOOP("Remove cookies for domain."), 0 },
 { "remove-all", I18N_NOOP("Remove all cookies."), 0 },
 { "reload-config", I18N_NOOP("Reload configuration file."), 0 },
 { 0, 0, 0 }
};

int main(int argc, char *argv[])
{
   KLocale::setMainCatalogue("kdelibs");
   KCmdLineArgs::init(argc, argv, "kcookiejar", description, version);

   KCmdLineArgs::addCmdLineOptions( options );

   KInstance a("kcookiejar");
   
   kapp->dcopClient()->attach();

   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   QCString replyType;
   QByteArray replyData;
   if (args->isSet("remove-all"))
   {
      kapp->dcopClient()->call( "kded", "kcookiejar", "deleteAllCookies()", QByteArray(), replyType, replyData);
   }
   if (args->isSet("remove"))
   {
      QString domain = args->getOption("remove");
      QByteArray params;
      QDataStream stream(params, IO_WriteOnly);
      stream << domain;
      kapp->dcopClient()->call( "kded", "kcookiejar", "deleteCookiesFromDomain", params, replyType, replyData);
   }
   if (args->isSet("shutdown"))
   {
      QCString module = "kcookiejar";
      QByteArray params;
      QDataStream stream(params, IO_WriteOnly);
      stream << module;
      kapp->dcopClient()->call( "kded", "kded", "unloadModule(QCString)", params, replyType, replyData);
   }
   else if(args->isSet("reload-config"))
   {
      kapp->dcopClient()->call( "kded", "kcookiejar", "reloadPolicy()", QByteArray(), replyType, replyData);
   }
   else
   {
      QCString module = "kcookiejar";
      QByteArray params;
      QDataStream stream(params, IO_WriteOnly);
      stream << module;
      kapp->dcopClient()->call( "kded", "kded", "loadModule(QCString)", params, replyType, replyData);
   }

   return 0;
}
