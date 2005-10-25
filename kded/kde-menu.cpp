/*  This file is part of the KDE libraries
 *  Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <stdlib.h>

#include <qfile.h>

#include <dcopclient.h>
#include <dcopref.h>

#include "kaboutdata.h"
#include "kapplication.h"
#include "kcmdlineargs.h"
#include "kglobal.h"
#include "klocale.h"
#include "kservice.h"
#include "kservicegroup.h"
#include "kstandarddirs.h"

static KCmdLineOptions options[] = {
   { "utf8", I18N_NOOP("Output data in UTF-8 instead of local encoding"), 0 },
   { "print-menu-id", I18N_NOOP("Print menu-id of the menu that contains\nthe application"), 0 },
   { "print-menu-name", I18N_NOOP("Print menu name (caption) of the menu that\ncontains the application"), 0 },
   { "highlight", I18N_NOOP("Highlight the entry in the menu"), 0 },
   { "nocache-update", I18N_NOOP("Do not check if sycoca database is up to date"), 0 },
   { "+<application-id>", I18N_NOOP("The id of the menu entry to locate"), 0 },
   KCmdLineLastOption
};

static const char appName[] = "kde-menu";
static const char appVersion[] = "1.0";
static bool utf8;

static bool bPrintMenuId;
static bool bPrintMenuName;
static bool bHighlight;

static void result(const QString &txt)
{
   if (utf8)
      puts( txt.toUtf8() );
   else
      puts( txt.toLocal8Bit() );
}

static void error(int exitCode, const QString &txt)
{
   qWarning("kde-menu: %s", txt.toLocal8Bit().data());
   exit(exitCode);
}

static void findMenuEntry(KServiceGroup::Ptr parent, const QString &name, const QString &menuId)
{
   KServiceGroup::List list = parent->entries(true, true, false);
   KServiceGroup::List::ConstIterator it = list.begin();
   for (; it != list.end(); ++it)
   {
      KSycocaEntry::Ptr e = (*it);

      if (e->isType(KST_KServiceGroup))
      {
         KServiceGroup::Ptr g = e;

         findMenuEntry(g, name.isEmpty() ? g->caption() : name+"/"+g->caption(), menuId);
      }
      else if (e->isType(KST_KService))
      {
         KService::Ptr s = e;
         if (s->menuId() == menuId)
         {
            if (bPrintMenuId)
            {
               result(parent->relPath());
            }
            if (bPrintMenuName)
            {
               result(name);
            }
            if (bHighlight)
            {
               DCOPRef kicker( "kicker", "kicker" );
               bool result = kicker.call( "highlightMenuItem", menuId );
               if (!result)
                  error(3, i18n("Menu item '%1' could not be highlighted.").arg(menuId).toLocal8Bit());
            }
            exit(0);
         }
      }
   }
}


int main(int argc, char **argv)
{
   KLocale::setMainCatalog("kdelibs");
   const char *description = I18N_NOOP("KDE Menu query tool.\n"
   "This tool can be used to find in which menu a specific application is shown.\n"
   "The --highlight option can be used to visually indicate to the user where\n"
   "in the KDE menu a specific application is located.");

   KAboutData d(appName, I18N_NOOP("kde-menu"), appVersion,
                description,
                KAboutData::License_GPL, "(c) 2003 Waldo Bastian");
   d.addAuthor("Waldo Bastian", I18N_NOOP("Author"), "bastian@kde.org");

   KCmdLineArgs::init(argc, argv, &d);
   KCmdLineArgs::addCmdLineOptions(options);

//   KApplication k(false, false);
   KApplication k(false);
   k.disableSessionManagement();

   // this program is in kdelibs so it uses kdelibs as catalog
   KLocale::setMainCatalog("kdelibs");

   KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
   if (args->count() != 1)
      KCmdLineArgs::usage(i18n("You must specify an application-id such as 'kde-konsole.desktop'"));

   utf8 = args->isSet("utf8");

   bPrintMenuId = args->isSet("print-menu-id");
   bPrintMenuName = args->isSet("print-menu-name");
   bHighlight = args->isSet("highlight");

   if (!bPrintMenuId && !bPrintMenuName && !bHighlight)
      KCmdLineArgs::usage(i18n("You must specify at least one of --print-menu-id, --print-menu-name or --highlight"));

   if (args->isSet("cache-update"))
   {
      QStringList args;
      args.append("--incremental");
      args.append("--checkstamps");
      QString command = "kbuildsycoca";
      QByteArray _launcher = KApplication::launcher();
      if (!DCOPRef(_launcher, _launcher).call("kdeinit_exec_wait", command, args).isValid())
      {
         qWarning("Can't talk to klauncher!");
         command = KGlobal::dirs()->findExe(command);
         command += " " + args.join(" ");
         system(command.toLocal8Bit());
      }
   }

   QString menuId = QFile::decodeName(args->arg(0));
   KService::Ptr s = KService::serviceByMenuId(menuId);

   if (!s)
      error(1, i18n("No menu item '%1'.").arg(menuId));

   findMenuEntry(KServiceGroup::root(), "", menuId);

   error(2, i18n("Menu item '%1' not found in menu.").arg(menuId));
   return 2;
}

