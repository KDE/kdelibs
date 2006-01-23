// -*- c-basic-offset: 3 -*-
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
   m_menuIdDictOffset = 0;
   if (m_str)
   {
      // Read Header
      qint32 i;
      (*m_str) >> i;
      m_nameDictOffset = i;
      (*m_str) >> i;
      m_relNameDictOffset = i;
      (*m_str) >> i;
      m_offerListOffset = i;
      (*m_str) >> i;
      m_initListOffset = i;
      (*m_str) >> i;
      m_menuIdDictOffset = i;

      int saveOffset = m_str->device()->at();
      // Init index tables
      m_nameDict = new KSycocaDict(m_str, m_nameDictOffset);
      // Init index tables
      m_relNameDict = new KSycocaDict(m_str, m_relNameDictOffset);
      // Init index tables
      m_menuIdDict = new KSycocaDict(m_str, m_menuIdDictOffset);
      saveOffset = m_str->device()->at(saveOffset);
   }
   else
   {
      // Build new database
      m_nameDict = new KSycocaDict();
      m_relNameDict = new KSycocaDict();
      m_menuIdDict = new KSycocaDict();
   }
   _self = this;
}

KServiceFactory::~KServiceFactory()
{
   _self = 0L;
   delete m_nameDict;
   delete m_relNameDict;
   delete m_menuIdDict;
}

KServiceFactory * KServiceFactory::self()
{
    if (!_self) {
        _self = new KServiceFactory();
    }
    return _self;
}

KService::Ptr KServiceFactory::findServiceByName(const QString &_name)
{
   if (!m_sycocaDict) return KService::Ptr(); // Error!

   // Warning : this assumes we're NOT building a database
   // But since findServiceByName isn't called in that case...
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_sycocaDict->find_string( _name );
   if (!offset) return KService::Ptr(); // Not found

   KService::Ptr newService(createEntry(offset));

   // Check whether the dictionary was right.
   if (newService && (newService->name() != _name))
   {
      // No it wasn't...
      return KService::Ptr();
   }
   return newService;
}

KService::Ptr KServiceFactory::findServiceByDesktopName(const QString &_name)
{
   if (!m_nameDict) return KService::Ptr(); // Error!

   // Warning : this assumes we're NOT building a database
   // But since this method isn't called in that case, we should be fine.
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_nameDict->find_string( _name );
   if (!offset) return KService::Ptr(); // Not found

   KService::Ptr newService(createEntry(offset));

   // Check whether the dictionary was right.
   if (newService && (newService->desktopEntryName() != _name))
   {
      // No it wasn't...
      return KService::Ptr();
   }
   return newService;
}

KService::Ptr KServiceFactory::findServiceByDesktopPath(const QString &_name)
{
   if (!m_relNameDict) return KService::Ptr(); // Error!

   // Warning : this assumes we're NOT building a database
   // But since this method isn't called in that case, we should be fine.
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_relNameDict->find_string( _name );
   if (!offset) {
      qDebug( "findServiceByDesktopPath: %s not found", qPrintable( _name ) );
      return KService::Ptr(); // Not found
   }

   KService::Ptr newService(createEntry(offset));
   if ( !newService )
      qDebug( "findServiceByDesktopPath: createEntry failed!" );
   // Check whether the dictionary was right.
   if (newService && (newService->desktopEntryPath() != _name))
   {
       qDebug( "the dictionary was wrong. desktopEntryPath=%s, name=%s", qPrintable( newService->desktopEntryPath() ), qPrintable( _name ) );
      // No it wasn't...
      return KService::Ptr();
   }
   return newService;
}

KService::Ptr KServiceFactory::findServiceByMenuId(const QString &_menuId)
{
   if (!m_menuIdDict) return KService::Ptr(); // Error!

   // Warning : this assumes we're NOT building a database
   // But since this method isn't called in that case, we should be fine.
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_menuIdDict->find_string( _menuId );
   if (!offset) return KService::Ptr(); // Not found

   KService::Ptr newService(createEntry(offset));

   // Check whether the dictionary was right.
   if (newService && (newService->menuId() != _menuId))
   {
      // No it wasn't...
      return KService::Ptr();
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
   const KSycocaEntry::List list = allEntries();
   KSycocaEntry::List::const_iterator it = list.begin();
   const KSycocaEntry::List::const_iterator end = list.end();
   for( ; it != end; ++it )
   {
      const KSycocaEntry::Ptr entry = *it;
      if ( entry->isType( KST_KService ) )
         result.append( KService::Ptr::staticCast( entry ) );
   }
   return result;
}

KService::List KServiceFactory::allInitServices()
{
   KService::List list;
   if (!m_str) return list;

   // Assume we're NOT building a database

   m_str->device()->at(m_initListOffset);
   qint32 entryCount;
   (*m_str) >> entryCount;

   // offsetList is needed because createEntry() modifies the stream position
   qint32 *offsetList = new qint32[entryCount];
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

   qint32 aServiceTypeOffset;
   qint32 aServiceOffset;
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
            const int savedPos = str->device()->at();
            // Create Service
            KService * serv = createEntry( aServiceOffset );
            if (serv) {
                list.append( KService::Ptr( serv ) );
            }
            // Restore position
            str->device()->at( savedPos );
         } else if ( aServiceTypeOffset > (qint32)serviceTypeOffset )
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

