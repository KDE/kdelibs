/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure <faure@kde.org>
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

#include "kservicefactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocadict.h"
#include "kservice.h"

#include <qstring.h>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

KServiceFactory::KServiceFactory()
 : KSycocaFactory( KST_KServiceFactory )
{
   m_offerListOffset = 0;
   m_nameDictOffset = 0;
   m_relNameDictOffset = 0;
   if (m_str)
   {
      // Read Header
      Q_INT32 i;
      (*m_str) >> i;
      m_nameDictOffset = i;
      (*m_str) >> i;
      m_relNameDictOffset = i;
      (*m_str) >> i;
      m_offerListOffset = i;
      (*m_str) >> i;
      m_initListOffset = i;

      int saveOffset = m_str->device()->at();
      // Init index tables
      m_nameDict = new KSycocaDict(m_str, m_nameDictOffset);
      // Init index tables
      m_relNameDict = new KSycocaDict(m_str, m_relNameDictOffset);
      saveOffset = m_str->device()->at(saveOffset);
   }
   else
   {
      // Build new database
      m_nameDict = new KSycocaDict();
      m_relNameDict = new KSycocaDict();
   }
   _self = this;
}

KServiceFactory::~KServiceFactory()
{
   _self = 0L;
   delete m_nameDict;
   delete m_relNameDict;
}

KServiceFactory * KServiceFactory::self()
{
    if (!_self) {
        _self = new KServiceFactory();
    }
    return _self;
}

KService * KServiceFactory::findServiceByName(const QString &_name)
{
   if (!m_sycocaDict) return 0; // Error!

   // Warning : this assumes we're NOT building a database
   // But since findServiceByName isn't called in that case...
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_sycocaDict->find_string( _name );
   if (!offset) return 0; // Not found

   KService * newService = createEntry(offset);

   // Check whether the dictionary was right.
   if (newService && (newService->name() != _name))
   {
      // No it wasn't...
      delete newService;
      newService = 0; // Not found
   }
   return newService;
}

KService * KServiceFactory::findServiceByDesktopName(const QString &_name)
{
   if (!m_nameDict) return 0; // Error!

   // Warning : this assumes we're NOT building a database
   // But since findServiceByName isn't called in that case...
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_nameDict->find_string( _name );
   if (!offset) return 0; // Not found

   KService * newService = createEntry(offset);

   // Check whether the dictionary was right.
   if (newService && (newService->desktopEntryName() != _name))
   {
      // No it wasn't...
      delete newService;
      newService = 0; // Not found
   }
   return newService;
}

KService * KServiceFactory::findServiceByDesktopPath(const QString &_name)
{
   if (!m_relNameDict) return 0; // Error!

   // Warning : this assumes we're NOT building a database
   // But since findServiceByName isn't called in that case...
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_relNameDict->find_string( _name );
   if (!offset) return 0; // Not found

   KService * newService = createEntry(offset);

   // Check whether the dictionary was right.
   if (newService && (newService->desktopEntryPath() != _name))
   {
      // No it wasn't...
      delete newService;
      newService = 0; // Not found
   }
   return newService;
}

KService* KServiceFactory::createEntry(int offset)
{
   KService * newEntry = 0L;
   KSycocaType type;
   QDataStream *str = KSycoca::self()->findEntry(offset, type);
   switch(type)
   {
     case KST_KService:
        newEntry = new KService(*str, offset);
        break;

     default:
        kdError(7011) << QString("KServiceFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type) << endl;
        return 0;
   }
   if (!newEntry->isValid())
   {
      kdError(7011) << "KServiceFactory: corrupt object in KSycoca database!\n" << endl;
      delete newEntry;
      newEntry = 0;
   }
   return newEntry;
}

KService::List KServiceFactory::allServices()
{
   KService::List result;
   KSycocaEntry::List list = allEntries();
   for( KSycocaEntry::List::Iterator it = list.begin();
        it != list.end();
        ++it)
   {
      KService *newService = dynamic_cast<KService *>((*it).data());
      if (newService)
         result.append( KService::Ptr( newService ) );
   }
   return result;
}

KService::List KServiceFactory::allInitServices()
{
   KService::List list;
   if (!m_str) return list;

   // Assume we're NOT building a database

   m_str->device()->at(m_initListOffset);
   Q_INT32 entryCount;
   (*m_str) >> entryCount;

   Q_INT32 *offsetList = new Q_INT32[entryCount];
   for(int i = 0; i < entryCount; i++)
   {
      (*m_str) >> offsetList[i];
   }

   for(int i = 0; i < entryCount; i++)
   {
      KService *newEntry = createEntry(offsetList[i]);
      if (newEntry)
      {
         list.append( KService::Ptr( newEntry ) );
      }
   }
   delete [] offsetList;
   return list;
}

KService::List KServiceFactory::offers( int serviceTypeOffset )
{
   KService::List list;

   QDataStream *str = m_str;
   // Jump to the offer list
   str->device()->at( m_offerListOffset );

   Q_INT32 aServiceTypeOffset;
   Q_INT32 aServiceOffset;
   // We might want to do a binary search instead of a linear search
   // since servicetype offsets are sorted. Bah.
   while (true)
   {
      (*str) >> aServiceTypeOffset;
      if ( aServiceTypeOffset )
      {
         (*str) >> aServiceOffset;
         if ( aServiceTypeOffset == serviceTypeOffset )
         {
            // Save stream position !
            int savedPos = str->device()->at();
            // Create Service
            KService * serv = createEntry( aServiceOffset );
            if (serv)
                list.append( KService::Ptr( serv ) );
            // Restore position
            str->device()->at( savedPos );
         } else if ( aServiceTypeOffset > (Q_INT32)serviceTypeOffset )
            break; // too far
      }
      else
         break; // 0 => end of list
   }
   return list;
}

KServiceFactory *KServiceFactory::_self = 0;

void KServiceFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }

