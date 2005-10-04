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
  m_serviceDict(977),
  m_dupeDict(977),
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

KService * KBuildServiceFactory::findServiceByName(const QString &_name)
{
   return m_serviceDict[_name];
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

  if (name.isEmpty())
     return 0;

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

   str << (Q_INT32) m_nameDictOffset;
   str << (Q_INT32) m_relNameDictOffset;
   str << (Q_INT32) m_offerListOffset;
   str << (Q_INT32) m_initListOffset;
   str << (Q_INT32) m_menuIdDictOffset;
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
   saveInitList(str);

   m_menuIdDictOffset = str.device()->at();
   m_menuIdDict->save(str);

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

   bool isNumber;
   for(KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
       itserv != m_entryDict->end();
       ++itserv)
   {
      KService *service = (KService *) ((KSycocaEntry *)(*itserv));
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
      // export associated services
      KServiceType *entry = static_cast<KServiceType*>(static_cast<KSycocaEntry*>(*it));
      KService::List services = entry->services();
  
      for(KService::List::ConstIterator it2 = services.begin();
          it2 != services.end(); ++it2)
      {
         KService *service = *it2;
         str << (Q_INT32) entry->offset();
         str << (Q_INT32) service->offset();
      }
   }

   str << (Q_INT32) 0;               // End of list marker (0)
}

void
KBuildServiceFactory::saveInitList(QDataStream &str)
{
   m_initListOffset = str.device()->at();

   KService::List initList;

   for(KSycocaEntryDict::Iterator itserv = m_entryDict->begin();
       itserv != m_entryDict->end();
       ++itserv)
   {
      KService::Ptr service = (KService *) ((KSycocaEntry *) *itserv);
      if ( !service->init().isEmpty() )
      {
          initList.append(service); 
      }
   }
   str << (Q_INT32) initList.count(); // Nr of init services.
   for(KService::List::Iterator it = initList.begin();
       it != initList.end();
       ++it)
   {
      str << (Q_INT32) (*it)->offset();
   }
}

void
KBuildServiceFactory::addEntry(KSycocaEntry *newEntry, const char *resource)
{
   if (m_dupeDict.find(newEntry))
      return;

   KSycocaFactory::addEntry(newEntry, resource);

   KService * service = (KService *) newEntry;
   m_dupeDict.insert(newEntry, service);

   if (!service->isDeleted())
   {
      QString parent = service->parentApp();
      if (!parent.isEmpty())
         m_serviceGroupFactory->addNewChild(parent, resource, service);
   }

   QString name = service->desktopEntryName();
   m_nameDict->add( name, newEntry );
   m_serviceDict.replace(name, service);

   QString relName = service->desktopEntryPath();
   m_relNameDict->add( relName, newEntry );
   QString menuId = service->menuId();
   if (!menuId.isEmpty())
      m_menuIdDict->add( menuId, newEntry );
}
