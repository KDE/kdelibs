/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure   <faure@kde.org>
 *                1999 Waldo Bastian <bastian@kde.org>
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

#include "stdio.h"

#include "kbuildservicefactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"

#include <kglobal.h>
#include <kstddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>

KBuildServiceFactory::KBuildServiceFactory( KSycocaFactory *serviceTypeFactory ) :
  KServiceFactory(), 
  m_serviceTypeFactory( serviceTypeFactory)
{
   m_resourceList->append( "apps" );
   m_resourceList->append( "services" );
}

KSycocaEntry * 
KBuildServiceFactory::createEntry( const QString& file, const char *resource )
{
  QString name = file;
  int pos = name.findRev('/');
  if (pos != -1)
  {
     name = name.mid(pos+1);
  }

  if (name.isEmpty())
     return 0;

  // Just a backup file ?
  if ( ( name.right(1) == "~") || 
       ( name.right(4) == ".bak") || 
       ( name[0] == '.') ||
       ( name[0] == '%' && name.right(1) == "%" ) )
      return 0;

  KDesktopFile desktopFile(file, true, resource);

  KService * serv = new KService( &desktopFile );

  if ( serv->isValid() )
     return serv;
  else {
     kDebugWarning( 7012, "Invalid Service : %s", file.ascii() );
     delete serv;
     return 0L;
  }
}


void
KBuildServiceFactory::saveHeader(QDataStream &str)
{
   KSycocaFactory::saveHeader(str);
kdebug(KDEBUG_INFO, 7012, QString("KBuildServiceFactory m_offerListOffset = %1")
	.arg( m_offerListOffset, 8, 16) );

   str << (Q_INT32) m_nameDictOffset;
   str << (Q_INT32) m_relNameDictOffset;
   str << (Q_INT32) m_offerListOffset;
}

void
KBuildServiceFactory::save(QDataStream &str)
{
   KSycocaFactory::save(str);

   m_nameDictOffset = str.device()->at();
   m_nameDict->save(str);

   m_relNameDictOffset = str.device()->at();
   m_relNameDict->save(str);

   saveOfferList(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #3)
   saveHeader(str);

   // Seek to end.
   str.device()->at(endOfFactoryData);
}

void 
KBuildServiceFactory::saveOfferList(QDataStream &str)
{
   m_offerListOffset = str.device()->at();

   // For each entry in servicetypeFactory
   for(QDictIterator<KSycocaEntry> it ( *(m_serviceTypeFactory->entryDict()) );
       it.current(); 
       ++it)
   {
      // export associated services
      // This means looking for the service type in ALL services
      // This is SLOW. But it used to be done in every app (in KServiceTypeProfile)
      // Doing it here saves a lot of time to the clients
      QString serviceType = it.current()->name();
      for(QDictIterator<KSycocaEntry> itserv ( *m_entryDict );
          itserv.current(); 
          ++itserv)
      {
         if ( ((KService *)itserv.current())->hasServiceType( serviceType ) )
         {
            str << (Q_INT32) it.current()->offset();
            str << (Q_INT32) itserv.current()->offset();
            //kdebug(KDEBUG_INFO, 7020, QString("<< %1 %2")
            //       .arg(it.current()->offset(),8,16).arg(itserv.current()->offset(),8,16));
         }
      }
   }
   str << (Q_INT32) 0;               // End of list marker (0)
}

void 
KBuildServiceFactory::addEntry(KSycocaEntry *newEntry)
{
   KSycocaFactory::addEntry(newEntry);

   KService * service = (KService *) newEntry;

   QString name = service->desktopEntryName();
   m_nameDict->add( name, newEntry );

   QString relName = service->desktopEntryPath();
   m_relNameDict->add( relName, newEntry );
}
