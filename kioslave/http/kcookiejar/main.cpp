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

#include <QtDBus/QtDBus>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapplication.h>
#include "kcookieserverinterface.h"
#include "kdedinterface.h"

int main(int argc, char *argv[])
{
   KLocalizedString description = ki18n("HTTP Cookie Daemon");

   const char version[] = "1.0";

   KCmdLineArgs::init(argc, argv, "kcookiejar", "kdelibs4", ki18n("HTTP cookie daemon"), version, description);

   KCmdLineOptions options;
   options.add("shutdown", ki18n("Shut down cookie jar"));
   options.add("remove <domain>", ki18n("Remove cookies for domain"));
   options.add("remove-all", ki18n("Remove all cookies"));
   options.add("reload-config", ki18n("Reload configuration file"));

   KCmdLineArgs::addCmdLineOptions( options );

   KComponentData a("kio4");

   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

   org::kde::KCookieServer *kcookiejar = new org::kde::KCookieServer("org.kde.kded", "/modules/kcookiejar", QDBusConnection::sessionBus());
   if (args->isSet("remove-all"))
   {
      kcookiejar->deleteAllCookies();
   }
   if (args->isSet("remove"))
   {
      QString domain = args->getOption("remove");
      kcookiejar->deleteCookiesFromDomain(domain);
   }
   if (args->isSet("shutdown"))
   {
      org::kde::kded kded("org.kde.kded", "/kded", QDBusConnection::sessionBus());
      kded.unloadModule("kcookiejar");
   }
   else if(args->isSet("reload-config"))
   {
      kcookiejar->reloadPolicy();
   }
   else
   {
      org::kde::kded kded("org.kde.kded", "/kded", QDBusConnection::sessionBus());
      kded.loadModule("kcookiejar");
   }
   delete kcookiejar;

   return 0;
}
