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
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"
#include "kmimetype.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessageboxwrapper.h>
#include <klocale.h>
#include <kdebug.h>
#include <assert.h>
#include <qhash.h>

KBuildServiceFactory::KBuildServiceFactory( KSycocaFactory *serviceTypeFactory,
	KBuildServiceGroupFactory *serviceGroupFactory ) :
  KServiceFactory(),
  m_serviceDict(),
  m_dupeDict(),
  m_serviceTypeFactory( serviceTypeFactory ),
  m_serviceGroupFactory( serviceGroupFactory )
{
   m_resourceList = new KSycocaResourceList();
//   m_resourceList->add( "apps", "*.desktop" );
//   m_resourceList->add( "apps", "*.kdelnk" );
   m_resourceList->add( "services", "*.desktop" );
   m_resourceList->add( "services", "*.kdelnk" );
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

// Return a KService* (as in KServiceFactory); the serviceDict retains ownership.
KService * KBuildServiceFactory::findServiceByName(const QString &_name)
{
   return m_serviceDict.value( _name ).get();
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
  if (!name.endsWith(".desktop") && !name.endsWith(".kdelnk"))
      return 0;

  KDesktopFile desktopFile(file, true, resource);

  KService * serv = new KService( &desktopFile );

  if ( serv->isValid() && !serv->isDeleted() )
  {
     return serv;
  } else {
     if (!serv->isDeleted())
        kdWarning(7012) << "Invalid Service : " << file << endl;
     delete serv;
     return 0L;
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

void
KBuildServiceFactory::saveOfferList(QDataStream &str)
{
   m_offerListOffset = str.device()->pos();

   bool isNumber;
   for(KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
       itserv != m_entryDict->end();
       ++itserv)
   {
      KService::Ptr service = (*itserv);
      QStringList serviceTypeList = service->serviceTypes();
      KServiceType::List serviceTypes;
      QStringList::ConstIterator it = serviceTypeList.begin();
      for( ; it != serviceTypeList.end(); ++it )
      {
         (*it).toInt(&isNumber);
         if (isNumber)
            continue;

         KServiceType::Ptr serviceType = KServiceType::serviceType(*it);
         if (!serviceType)
         {
           kdWarning() << "'"<< service->desktopEntryPath() << "' specifies undefined mimetype/servicetype '"<< (*it) << "'" << endl;
           continue;
         }
         serviceTypes.append(serviceType);
      }

      while(serviceTypes.count())
      {
         KServiceType::Ptr serviceType = serviceTypes.first();
         serviceTypes.pop_front();

         KServiceType::Ptr parentType = serviceType->parentType();
         if (parentType)
            serviceTypes.append(parentType);

         serviceType->addService(service);
      }
   }

   // For each entry in servicetypeFactory
   for(KSycocaEntryDict::Iterator it = m_serviceTypeFactory->entryDict()->begin();
       it != m_serviceTypeFactory->entryDict()->end();
       ++it)
   {
       const KSycocaEntry* baseEntry = (*it).data();
      // export associated services
#ifdef __GNUC__
#warning I added this here, but it should not be 0 in the first place (coolo)
#endif
      if ( !baseEntry ) {
         kdDebug() << "no entry\n";
         continue;
      }
      const KServiceType *entry = static_cast<const KServiceType*>( baseEntry );
      const KService::List services = entry->services();

      for(KService::List::ConstIterator it2 = services.begin();
          it2 != services.end(); ++it2)
      {
         const KService::Ptr &service = (*it2);
         str << (qint32) entry->offset();
         str << (qint32) service->offset();
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
      const KService::Ptr& service = (*itserv);
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
KBuildServiceFactory::addEntry(KSycocaEntry::Ptr newEntry, const char *resource)
{
   Q_ASSERT( newEntry );
   if (m_dupeDict.contains(newEntry))
      return;

   KSycocaFactory::addEntry(newEntry, resource);

   KService::Ptr service = newEntry;
   m_dupeDict.insert(newEntry);

   if (!service->isDeleted())
   {
      QString parent = service->parentApp();
      if (!parent.isEmpty())
         m_serviceGroupFactory->addNewChild(parent, resource, service);
   }

   QString name = service->desktopEntryName();
   m_nameDict->add( name, newEntry );
   m_serviceDict.insert(name, service);

   QString relName = service->desktopEntryPath();
   m_relNameDict->add( relName, newEntry );
   QString menuId = service->menuId();
   if (!menuId.isEmpty())
      m_menuIdDict->add( menuId, newEntry );
}
