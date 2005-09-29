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

#include <kdebug.h>
#include <kuserprofile.h>
#include <ktrader.h>
#include <kservice.h>
#include <kmimetype.h>
#include <assert.h>
#include <kstandarddirs.h>
#include <kservicegroup.h>
#include <kimageio.h>
#include <kprotocolinfo.h>
#include <kiconloader.h>

#include <kapplication.h>

#include <stdio.h>
#include <stdlib.h>

bool check(QString txt, QString a, QString b)
{
  if (a.isEmpty())
     a = QString::null;
  if (b.isEmpty())
     b = QString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl
;
    exit(1);
  }
  return true;
}

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

int main(int argc, char *argv[])
{
   KApplication k(argc,argv,"whatever",false/*noGUI*/); // KMessageBox needs KApp for makeStdCaption

   QCString instname = "kword";
   QString desktopPath = QString::fromLatin1( "Office/%1.desktop" ).arg( instname );
   qDebug( "Looking for %s", desktopPath.latin1() );
   KService::Ptr service = KService::serviceByDesktopPath( desktopPath );
   if ( service )
       qDebug( "found: %s", service->desktopEntryPath().latin1() );
   else
       qDebug( "not found" );

   qDebug( "Looking for desktop name = %s", instname.data() );
   service = KService::serviceByDesktopName( instname );
   if ( service )
       qDebug( "found: %s", service->desktopEntryPath().latin1() );
   else
       qDebug( "not found" );

   debug("Trying to look for text/plain");
   KMimeType::Ptr s1 = KMimeType::mimeType("text/plain");
   if ( s1 )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(s1->comment(KURL(),false)));
   }
   else
   {
     debug("Not found !");
     exit(1);
   }

   debug("Trying to look for application/x-zerosize");
   KMimeType::Ptr s0 = KMimeType::mimeType("application/x-zerosize");
   if ( s0 )
   {
     debug("Found it !");
     debug(QString("Comment is %1").arg(s0->comment(KURL(),false)));
   }
   else
   {
     debug("Not found !");
     exit(1);
   }

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
   debug("\nTrying findByURL for $KDEDIR/bin/kdesktop");
   KMimeType::Ptr mf  = KMimeType::findByURL( KStandardDirs::findExe( "kdesktop" ), 0,
				 true, false );
   assert( mf );
   check( "A binary's mimetype", mf->name(), "application/x-executable" );

   //
   debug("\nTrying findByURL for folder_home.png");
   QString fh;
   (void)k.iconLoader()->loadIcon("folder_home.png",KIcon::Desktop,0,KIcon::DefaultState,&fh);
   mf  = KMimeType::findByURL( fh, 0, true, false );
   assert( mf );
   check( "A PNG's mimetype", mf->name(), "image/png" );

   //
   //debug("\nTrying findByURL for Makefile.am");
   //mf = KMimeType::findByURL( KURL("/tmp/Makefile.am"), 0, true, false );
   //assert( mf );
   //debug(QString("Name is %1").arg(mf->name()));
   //debug(QString("Comment is %1").arg(mf->comment(KURL(),false)));

   debug("\nTrying findByURL for man:/ls");
   mf = KMimeType::findByURL( KURL("man:/ls") );
   assert( mf );
   check( "man:/ls", mf->name(), "text/html" );
   check( "man:/ls/", mf->name(), "text/html" );

   mf = KMimeType::findByURL( KURL("http://foo/bar.png") );
   check( "HTTP URL", mf->name(), "application/octet-stream" ); // HTTP can't know before downloading

#if 1
   KMimeType::List mtl;

   mtl  = KMimeType::allMimeTypes( );
   assert( mtl.count() );
   debug(QString("Found %1 mime types.").arg(mtl.count()));
   for(int i = 0; i < (int)mtl.count(); i++)
   {
      debug(QString("Mime type %1: %2.").arg(i).arg(mtl[i]->name()));
   }

   KService::List sl;

   sl  = KService::allServices( );
   assert( sl.count() );
   debug(QString("Found %1 services.").arg(sl.count()));

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
      KSycocaEntry *p = (*it);
      if (p->isType(KST_KService))
      {
         KService *service = static_cast<KService *>(p);
         debug(service->name());
         debug(service->desktopEntryPath());
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p);
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
      KSycocaEntry *p = (*it);
      if (p->isType(KST_KService))
      {
         KService *service = static_cast<KService *>(p);
         debug(QString("             %1").arg(service->name()));
      }
      else if (p->isType(KST_KServiceGroup))
      {
         KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p);
         debug(QString("             %1 -->").arg(serviceGroup->caption()));
      }
      else
      {
         debug("KServiceGroup: Unexpected object in list!");
      }
   }
   }

   debug("--services that require initialisation--");
   sl = KService::allInitServices();
   for( KService::List::ConstIterator it = sl.begin();
       it != sl.end(); it++)
   {
      KService *service = static_cast<KService *>(*it);
      debug(service->name());
   }
   debug("--End of list--");

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
