/*
 *  Copyright (C) 2002, 2003 David Faure   <faure@kde.org>
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
#include <kimageio.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>
#include <ktrader.h>
#include <kuserprofile.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void debug(QString txt)
{
 fprintf(stderr, "%s\n", txt.ascii());
}

void debug(const char *txt)
{
 fprintf(stderr, "%s\n", txt);
}
void debug(const char *format, const char *txt)
{
 fprintf(stderr, format, txt);
 fprintf(stderr, "\n");
}

// ############
// Some of the tests here (those that don't depend on other modules being installed)
// should be moved to kmimetypetest, and then kmimetypetest can be renamed ksycocatest.
//
int main(int argc, char *argv[])
{
   KApplication::disableAutoDcopRegistration();
   KCmdLineArgs::init(argc,argv,"ksycocatest", 0, 0, 0, 0);
   KApplication k;//(argc,argv,"whatever",false/*noGUI*/); // KMessageBox needs KApp for makeStdCaption

   QString instname = "kword";
   QString desktopPath = QString::fromLatin1( "Office/%1.desktop" ).arg( instname );
   qDebug( "Looking for %s", desktopPath.latin1() );
   KService::Ptr service = KService::serviceByDesktopPath( desktopPath );
   if ( service )
       qDebug( "found: %s", service->desktopEntryPath().latin1() );
   else
       qDebug( "not found" );

   qDebug( "Looking for desktop name = %s", instname.latin1() );
   service = KService::serviceByDesktopName( instname );
   if ( service )
       qDebug( "found: %s", service->desktopEntryPath().latin1() );
   else
       qDebug( "not found" );

   debug("Trying to look for Desktop Pager");
   KService::Ptr se = KService::serviceByName("Desktop Pager");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }

   debug("Trying to look for kpager");
   se = KService::serviceByDesktopName("kpager");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
     QVariant qv = se->property("DocPath");
     debug(QString("Property type is %1").arg(qv.typeName()));
     debug(QString("Property value is %1").arg(qv.toString()));
   }
   else
   {
     debug("Not found !");
   }

   debug("Trying to look for System/kpager.desktop");
   se = KService::serviceByDesktopPath("System/kpager.desktop");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }

   debug("Trying to look for System/fake-entry.desktop");
   se = KService::serviceByDesktopPath("System/fake-entry.desktop");
   if ( se )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(se->comment()));
   }
   else
   {
     debug("Not found !");
   }

#if 1
   debug("Querying userprofile for services associated with text/plain");
   KServiceTypeProfile::OfferList offers = KServiceTypeProfile::offers("text/plain");
   debug(QString("got %1 offers").arg(offers.count()));
   KServiceTypeProfile::OfferList::Iterator it = offers.begin();
   for ( ; it != offers.end() ; it++ )
   {
     debug((*it).service()->name());
   }

   debug("Querying userprofile for services associated with KOfficeFilter");
   offers = KServiceTypeProfile::offers("KOfficeFilter");
   debug(QString("got %1 offers").arg(offers.count()));
   it = offers.begin();
   for ( ; it != offers.end() ; it++ )
   {
     debug((*it).service()->name());
   }

   debug("Querying trader for Konqueror/Plugin");
   KTrader::OfferList traderoffers = KTrader::self()->query("Konqueror/Plugin");
   debug(QString("got %1 offers").arg(traderoffers.count()));
   KTrader::OfferList::Iterator trit = traderoffers.begin();
   for ( ; trit != traderoffers.end() ; trit++ )
   {
     debug((*trit)->name());
   }
#endif

   //
   //debug("\nTrying findByURL for Makefile.am");
   //mf = KMimeType::findByURL( KURL("/tmp/Makefile.am"), 0, true, false );
   //assert( mf );
   //debug(QString("Name is %1").arg(mf->name()));
   //debug(QString("Comment is %1").arg(mf->comment(KURL(),false)));

#if 1
   KServiceGroup::Ptr root = KServiceGroup::root();
   KServiceGroup::List list = root->entries();
   //KServiceGroup::Ptr topGroup = KServiceGroup::childGroup( "kview" );
   //Q_ASSERT( topGroup );
   //KServiceGroup::List list = topGroup->entries();

   KServiceGroup::Ptr first;

   debug(QString("Found %1 entries").arg(list.count()));
   for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
   {
      KSycocaEntry::Ptr p = (*it);
      if (p->isType(KST_KService))
      {
          KService::Ptr service = KService::Ptr::staticCast( p );
         debug(service->name());
         debug(service->desktopEntryPath());
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);
         debug(QString("             %1 -->").arg(serviceGroup->caption()));
         if (!first) first = serviceGroup;
      }
      else
      {
         debug("KServiceGroup: Unexpected object in list!");
      }
   }

   if (first)
   {
   list = first->entries();
   debug(QString("Found %1 entries").arg(list.count()));
   for( KServiceGroup::List::ConstIterator it = list.begin();
       it != list.end(); it++)
   {
      KSycocaEntry::Ptr p = (*it);
      if (p->isType(KST_KService))
      {
         KService::Ptr service = KService::Ptr::staticCast( p );
         debug(QString("             %1").arg(service->name()));
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);
         debug(QString("             %1 -->").arg(serviceGroup->caption()));
      }
      else
      {
         debug("KServiceGroup: Unexpected object in list!");
      }
   }
   }

   debug("--protocols--");
   QStringList stringL = KProtocolInfo::protocols();
   for( QStringList::ConstIterator it = stringL.begin();
       it != stringL.end(); it++)
   {
      debug((*it).ascii());
   }
   debug("--End of list--");
#endif

#if 0
   KImageIO::registerFormats();

   QStringList types;
   types = KImageIO::types(KImageIO::Reading);
   debug("Can read:");
   for(QStringList::ConstIterator it = types.begin();
       it != types.end(); ++it)
      debug(QString("    %1").arg((*it)));

   types = KImageIO::types(KImageIO::Writing);
   debug("Can write:");
   for(QStringList::ConstIterator it = types.begin();
       it != types.end(); ++it)
      debug(QString("    %1").arg((*it)));


   QString rPattern = KImageIO::pattern( KImageIO::Reading );
   debug("Read pattern:\n%s", rPattern.ascii());

   QString wPattern = KImageIO::pattern( KImageIO::Writing );
   debug("Write pattern:\n%s", wPattern.ascii());

   QString suffix = KImageIO::suffix("JPEG");
   debug("Standard suffix for JPEG: %s", suffix.ascii());

   types = KImageIO::mimeTypes(KImageIO::Reading);
   debug("Can read (mimetypes):");
   for(QStringList::ConstIterator it = types.begin();
       it != types.end(); ++it)
      debug("    %s", (*it).ascii());

   types = KImageIO::mimeTypes(KImageIO::Writing);
   debug("Can write (mimetypes):");
   for(QStringList::ConstIterator it = types.begin();
       it != types.end(); ++it)
      debug("    %s", (*it).ascii());

   debug("done");
#endif
   return 0;
}
