/*
 *  Copyright (C) 2002-2006 David Faure   <faure@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>
#include <kmimetypetrader.h>
#include <kservicetypeprofile.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


// ############
// Some of the tests here (those that don't depend on other modules being installed)
// should be moved to kmimetypetest, and then kmimetypetest can be renamed ksycocatest.
//
// But it's still convenient to have an interactive test program, for checking things
// in the user's ksycoca instead of checking them in the safe .kde-unit-test one.
//
int main(int argc, char *argv[])
{
   //KApplication::disableAutoDcopRegistration();
   KCmdLineArgs::init(argc,argv, "ksycocatest", 0, ki18n("ksycocatest"), 0);
   KApplication k;//(argc,argv,"whatever",false/*noGUI*/); // KMessageBox needs KApp for makeStdCaption

   QString instname = "kword";
   QString desktopPath = QString::fromLatin1( "Office/%1.desktop" ).arg( instname );
   qDebug( "Looking for %s", desktopPath.toLatin1().constData() );
   KService::Ptr service = KService::serviceByDesktopPath( desktopPath );
   if ( service )
       qDebug( "found: %s", service->entryPath().toLatin1().constData() );
   else
       qDebug( "not found" );

   qDebug( "Looking for desktop name = %s", instname.toLatin1().constData() );
   service = KService::serviceByDesktopName( instname );
   if ( service )
       qDebug( "found: %s", service->entryPath().toLatin1().constData() );
   else
       qDebug( "not found" );

   qDebug("Trying to look for Desktop Pager");
   KService::Ptr se = KService::serviceByName("Desktop Pager");
   if ( se )
   {
     qDebug("Found it !");
     qDebug("Comment is %s", qPrintable(se->comment()));
   }
   else
   {
     qDebug("Not found !");
   }

   qDebug("Trying to look for kpager");
   se = KService::serviceByDesktopName("kpager");
   if ( se )
   {
     qDebug("Found it !");
     qDebug("Comment is %s", qPrintable(se->comment()));
     QVariant qv = se->property("X-DocPath");
     qDebug("Property type is %s", qv.typeName());
     qDebug("Property value is %s", qPrintable(qv.toString()));
   }
   else
   {
     qDebug("Not found !");
   }

   qDebug("Trying to look for System/kpager.desktop");
   se = KService::serviceByDesktopPath("System/kpager.desktop");
   if ( se )
   {
     qDebug("Found it !");
     qDebug("Comment is %s", qPrintable(se->comment()));
   }
   else
   {
     qDebug("Not found !");
   }

   qDebug("Trying to look for System/fake-entry.desktop");
   se = KService::serviceByDesktopPath("System/fake-entry.desktop");
   if ( se )
   {
     qDebug("Found it !");
     qDebug("Comment is %s", qPrintable(se->comment()));
   }
   else
   {
     qDebug("Not found !");
   }

#if 1
   KServiceGroup::Ptr root = KServiceGroup::root();
   KServiceGroup::List list = root->entries();
   //KServiceGroup::Ptr topGroup = KServiceGroup::childGroup( "kview" );
   //Q_ASSERT( topGroup );
   //KServiceGroup::List list = topGroup->entries();

   KServiceGroup::Ptr first;

   qDebug("Found %d entries", list.count());
   for( KServiceGroup::List::ConstIterator it = list.constBegin();
       it != list.constEnd(); ++it)
   {
      KSycocaEntry::Ptr p = (*it);
      if (p->isType(KST_KService))
      {
          KService::Ptr service = KService::Ptr::staticCast( p );
         qDebug("%s", qPrintable(service->name()));
         qDebug("%s", qPrintable(service->entryPath()));
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);
         qDebug("             %s -->", qPrintable(serviceGroup->caption()));
         if (!first) first = serviceGroup;
      }
      else
      {
         qDebug("KServiceGroup: Unexpected object in list!");
      }
   }

   if (first)
   {
   list = first->entries();
   qDebug("Found %d entries",list.count());
   for( KServiceGroup::List::ConstIterator it = list.constBegin();
       it != list.constEnd(); ++it)
   {
      KSycocaEntry::Ptr p = (*it);
      if (p->isType(KST_KService))
      {
         KService::Ptr service = KService::Ptr::staticCast( p );
         qDebug("             %s", qPrintable(service->name()));
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);
         qDebug("             %s -->", qPrintable(serviceGroup->caption()));
      }
      else
      {
         qDebug("KServiceGroup: Unexpected object in list!");
      }
   }
   }

   qDebug("--protocols--");
   QStringList stringL = KProtocolInfo::protocols();
   qDebug() << stringL;
#endif
   return 0;
}
