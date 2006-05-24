// -*- c-basic-offset: 3 -*-
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kbuildservicefactory.h"
#include "kbuildmimetypefactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"
#include "kmimetype.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <assert.h>
#include <qhash.h>
#include <kmimetypefactory.h>

KBuildServiceFactory::KBuildServiceFactory( KSycocaFactory *serviceTypeFactory,
                                            KBuildMimeTypeFactory *mimeTypeFactory,
                                            KBuildServiceGroupFactory *serviceGroupFactory ) :
  KServiceFactory(),
  m_serviceDict(),
  m_dupeDict(),
  m_serviceTypeFactory( serviceTypeFactory ),
  m_mimeTypeFactory( mimeTypeFactory ),
  m_serviceGroupFactory( serviceGroupFactory )
{
   m_resourceList = new KSycocaResourceList();
//   m_resourceList->add( "apps", "*.desktop" );
   m_resourceList->add( "services", "*.desktop" );
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildServiceFactory::resourceTypes()
{
    return QStringList() << "apps" << "services";
}

KBuildServiceFactory::~KBuildServiceFactory()
{
   delete m_resourceList;
}

KService::Ptr KBuildServiceFactory::findServiceByName(const QString &_name)
{
   return m_serviceDict.value( _name );
}


KSycocaEntry *
KBuildServiceFactory::createEntry( const QString& file, const char *resource )
{
  QString name = file;
  int pos = name.lastIndexOf('/');
  if (pos != -1)
  {
     name = name.mid(pos+1);
  }
  // Is it a .desktop file?
  if (!name.endsWith(".desktop"))
      return 0;

  KDesktopFile desktopFile(file, true, resource);

  KService * serv = new KService( &desktopFile );

  if ( serv->isValid() && !serv->isDeleted() )
  {
     return serv;
  } else {
     if (!serv->isDeleted())
        kWarning(7012) << "Invalid Service : " << file << endl;
     delete serv;
     return 0;
  }
}


void
KBuildServiceFactory::saveHeader(QDataStream &str)
{
   KSycocaFactory::saveHeader(str);

   str << (qint32) m_nameDictOffset;
   str << (qint32) m_relNameDictOffset;
   str << (qint32) m_offerListOffset;
   str << (qint32) m_initListOffset;
   str << (qint32) m_menuIdDictOffset;
}

void
KBuildServiceFactory::save(QDataStream &str)
{
   KSycocaFactory::save(str);

   m_nameDictOffset = str.device()->pos();
   m_nameDict->save(str);

   m_relNameDictOffset = str.device()->pos();
   m_relNameDict->save(str);

   saveOfferList(str);
   saveInitList(str);

   m_menuIdDictOffset = str.device()->pos();
   m_menuIdDict->save(str);

   int endOfFactoryData = str.device()->pos();

   // Update header (pass #3)
   saveHeader(str);


   // Seek to end.
   str.device()->seek(endOfFactoryData);
}

void KBuildServiceFactory::populateServiceTypes()
{
   kDebug() << k_funcinfo << endl;
   bool isNumber;
   KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
   const KSycocaEntryDict::Iterator endserv = m_entryDict->end();
   for( ; itserv != endserv ; ++itserv )
   {
      const KService::Ptr service = KService::Ptr::staticCast(*itserv);
      const QStringList serviceTypeList = service->serviceTypes();
      //kDebug(7021) << "service " << service->desktopEntryPath() << " has serviceTypes " << serviceTypeList << endl;
      QMap<KServiceType::Ptr,int> serviceTypes; // with preference number
      QListIterator<QString> it( serviceTypeList );
      while ( it.hasNext() )
      {
         QString str = it.next();
         KServiceType::Ptr serviceType = KServiceType::serviceType(str);
         if (!serviceType) {
            // Note: don't ever use KMimeType::mimeType() in kbuildsycoca, since it falls back to application/octet-stream
            serviceType = KServiceType::Ptr::staticCast( m_mimeTypeFactory->findMimeTypeByName( str ) );
         }
         if (!serviceType)
         {
           kWarning() << "'"<< service->desktopEntryPath() << "' specifies undefined mimetype/servicetype '"<< str << "'" << endl;
           continue;
         }

         int initialPreference = service->initialPreference();
         if ( it.hasNext() )
         {
            const int val = it.peekNext().toInt(&isNumber);
            if (isNumber) {
               initialPreference = val;
               it.next();
            }
         }
         serviceTypes.insert(serviceType, initialPreference);
      }

      // Add this service to all its servicetypes (and their parents)
      while(!serviceTypes.isEmpty()) // can't use foreach due to append inside the loop
      {
         QMap<KServiceType::Ptr,int>::iterator it = serviceTypes.begin();
         KServiceType::Ptr serviceType = it.key();
         const QString serviceTypeName = serviceType->name();
         const int initialPreference = it.value();
         serviceTypes.erase( it );

         KServiceType::Ptr parentType = serviceType->parentType();
         if (parentType)
            serviceTypes.insert(parentType, initialPreference);

         //kDebug(7021) << "Adding service " << service->desktopEntryPath() << " to " << serviceType->name() << endl;
         addServiceOffer( serviceTypeName, service, initialPreference );
      }
   }

   // Now for each mimetype, collect services from parent mimetypes
   const KMimeType::List allMimeTypes = m_mimeTypeFactory->allMimeTypes();
   KMimeType::List::const_iterator itm = allMimeTypes.begin();
   for( ; itm != allMimeTypes.end(); ++itm )
   {
      const KMimeType::Ptr mimeType = *itm;
      const QString mimeTypeName = mimeType->name();
      QString parent = mimeType->parentMimeType();
      if ( parent.isEmpty() )
         continue;
      while ( !parent.isEmpty() )
      {
         const KMimeType::Ptr parentMimeType = m_mimeTypeFactory->findMimeTypeByName( parent );
         if ( parentMimeType ) {
            const QMap<KService::Ptr,int>& lst = m_serviceTypeData[parent].data;
            QMap<KService::Ptr,int>::const_iterator itserv = lst.begin();
            const QMap<KService::Ptr,int>::const_iterator endserv = lst.end();
            for ( ; itserv != endserv; ++itserv ) {
               addServiceOffer( mimeTypeName, itserv.key(), itserv.value() );
            }
            parent = parentMimeType->parentMimeType();
         } else {
            kWarning(7012) << "parent mimetype not found:" << parent << endl;
            break;
         }
      }
   }
   // TODO do the same for all/all and all/allfiles, if (!KServiceTypeProfile::configurationMode())

   // Now collect the offsets into the (future) offer list
   // The loops look very much like the ones in saveOfferList obviously.
   int offersOffset = 0;
   const int offerEntrySize = sizeof( qint32 ) * 3; // three qint32s, see saveOfferList.

   KSycocaEntryDict::const_iterator itstf = m_serviceTypeFactory->entryDict()->begin();
   const KSycocaEntryDict::const_iterator endstf = m_serviceTypeFactory->entryDict()->end();
   for( ; itstf != endstf; ++itstf ) {
      KServiceType::Ptr entry = KServiceType::Ptr::staticCast( *itstf );
      const int numOffers = m_serviceTypeData[entry->name()].data.count();
      if ( numOffers ) {
         entry->setServiceOffersOffset( offersOffset );
         offersOffset += offerEntrySize * numOffers;
      }
   }
   KSycocaEntryDict::const_iterator itmtf = m_mimeTypeFactory->entryDict()->begin();
   const KSycocaEntryDict::const_iterator endmtf = m_mimeTypeFactory->entryDict()->end();
   for( ; itmtf != endmtf; ++itmtf )
   {
      KMimeType::Ptr entry = KMimeType::Ptr::staticCast( *itmtf );
      const int numOffers = m_serviceTypeData[entry->name()].data.count();
      if ( numOffers ) {
         entry->setServiceOffersOffset( offersOffset );
         offersOffset += offerEntrySize * numOffers;
      }
   }
}

void KBuildServiceFactory::saveOfferList(QDataStream &str)
{
   m_offerListOffset = str.device()->pos();

   // For each entry in servicetypeFactory
   KSycocaEntryDict::const_iterator itstf = m_serviceTypeFactory->entryDict()->begin();
   const KSycocaEntryDict::const_iterator endstf = m_serviceTypeFactory->entryDict()->end();
   for( ; itstf != endstf; ++itstf )
   {
      // export associated services
      const KServiceType::Ptr entry = KServiceType::Ptr::staticCast( *itstf );
      Q_ASSERT( entry );

      const QMap<KService::Ptr,int> services = m_serviceTypeData[entry->name()].data;

      for(QMap<KService::Ptr,int>::const_iterator it2 = services.begin();
          it2 != services.end(); ++it2)
      {
         //kDebug(7021) << "servicetype offers list: " << entry->name() << " -> " << it2.key()->desktopEntryPath() << endl;

         str << (qint32) entry->offset();
         str << (qint32) it2.key()->offset(); // service offset
         str << (qint32) it2.value(); // initial preference
      }
   }

   // For each entry in mimeTypeFactory
   KSycocaEntryDict::const_iterator itmtf = m_mimeTypeFactory->entryDict()->begin();
   const KSycocaEntryDict::const_iterator endmtf = m_mimeTypeFactory->entryDict()->end();
   for( ; itmtf != endmtf; ++itmtf )
   {
      // export associated services
      const KMimeType::Ptr entry = KMimeType::Ptr::staticCast( *itmtf );
      Q_ASSERT( entry );
      const QMap<KService::Ptr,int> services = m_serviceTypeData[entry->name()].data;

      for(QMap<KService::Ptr,int>::const_iterator it2 = services.begin();
          it2 != services.end(); ++it2)
      {
         //kDebug(7021) << "mimetype offers list: " << entry->name() << " -> " << it2.key()->desktopEntryPath() << endl;

         str << (qint32) entry->offset();
         str << (qint32) it2.key()->offset(); // service offset
         str << (qint32) it2.value(); // initial preference
      }
   }

   str << (qint32) 0;               // End of list marker (0)
}

void
KBuildServiceFactory::saveInitList(QDataStream &str)
{
   m_initListOffset = str.device()->pos();

   KService::List initList;

   for(KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
       itserv != m_entryDict->end();
       ++itserv)
   {
      const KService::Ptr service = KService::Ptr::staticCast(*itserv);
      if ( !service->init().isEmpty() )
      {
          initList.append(service);
      }
   }
   str << (qint32) initList.count(); // Nr of init services.
   for(KService::List::const_iterator it = initList.begin();
       it != initList.end();
       ++it)
   {
      str << (qint32) (*it)->offset();
   }
}

void
KBuildServiceFactory::addEntry(const KSycocaEntry::Ptr& newEntry)
{
   Q_ASSERT(newEntry);
   if (m_dupeDict.contains(newEntry))
      return;

   KSycocaFactory::addEntry(newEntry);

   const KService::Ptr service = KService::Ptr::staticCast( newEntry );
   m_dupeDict.insert(newEntry);

   if (!service->isDeleted())
   {
      QString parent = service->parentApp();
      if (!parent.isEmpty())
         m_serviceGroupFactory->addNewChild(parent, KSycocaEntry::Ptr::staticCast(service));
   }

   const QString name = service->desktopEntryName();
   m_nameDict->add( name, newEntry );
   m_serviceDict.insert(name, service);

   const QString relName = service->desktopEntryPath();
   m_relNameDict->add( relName, newEntry );
   const QString menuId = service->menuId();
   if (!menuId.isEmpty())
      m_menuIdDict->add( menuId, newEntry );
}

void KBuildServiceFactory::addServiceOffer( const QString& serviceType, const KService::Ptr& service, int initialPreference )
{
   //kDebug(7021) << "Adding " << service.data() << " to " << serviceType << endl;
   QMap<KService::Ptr,int>& data = m_serviceTypeData[serviceType].data;
   if ( data.find( service ) == data.end() ) {
      int c = data.count();
      data.insert( service, initialPreference );
      Q_ASSERT( data.count() == c + 1 );
   }
   //else
   //   kDebug(7021) << service.data() << " " << service->desktopEntryPath() << " already in " << serviceType << endl;
}
