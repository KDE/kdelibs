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

#include "kbuildservicefactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstddirs.h>
#include <kmessageboxwrapper.h>
#include <klocale.h>
#include <kdebug.h>
#include <assert.h>

KBuildServiceFactory::KBuildServiceFactory( KSycocaFactory *serviceTypeFactory,
	KBuildServiceGroupFactory *serviceGroupFactory ) :
  KServiceFactory(),
  m_serviceTypeFactory( serviceTypeFactory ),
  m_serviceGroupFactory( serviceGroupFactory )
{
   m_resourceList = new KSycocaResourceList();
   m_resourceList->add( "apps", "*.desktop" );
   m_resourceList->add( "apps", "*.kdelnk" );
   m_resourceList->add( "services", "*.desktop" );
   m_resourceList->add( "services", "*.kdelnk" );
}

KBuildServiceFactory::~KBuildServiceFactory()
{
   delete m_resourceList;
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

  if ( name == ".directory")
  {
     m_serviceGroupFactory->addNewEntry(file, resource, 0);
     return 0;
  }

  // Is it a .desktop file?
  if ((name.right(8) != ".desktop") && (name.right(7) != ".kdelnk"))
      return 0;

  KDesktopFile desktopFile(file, true, resource);

  KService * serv = new KService( &desktopFile );

  if ( serv->isValid() ) 
  {
     return serv;
  } else {
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
         }
      }
   }
   str << (Q_INT32) 0;               // End of list marker (0)
}

void
KBuildServiceFactory::addEntry(KSycocaEntry *newEntry, const char *resource)
{
   KSycocaFactory::addEntry(newEntry, resource);

   KService * service = (KService *) newEntry;
   if (!service->isDeleted())
      m_serviceGroupFactory->addNewEntry(service->entryPath(), resource, service);

   QString name = service->desktopEntryName();
   m_nameDict->add( name, newEntry );

   QString relName = service->desktopEntryPath();
   m_relNameDict->add( relName, newEntry );
}
