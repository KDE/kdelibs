/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
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

#include "kservicetypefactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocadict.h"
#include "kservicetype.h"
#include "kmimetype.h"
#include "kuserprofile.h"

#include <kapplication.h>
#include <kdebug.h>
#include <assert.h>
#include <kstringhandler.h>
#include <qfile.h>

KServiceTypeFactory::KServiceTypeFactory()
 : KSycocaFactory( KST_KServiceTypeFactory )
{
   _self = this;
   m_fastPatternOffset = 0;
   m_otherPatternOffset = 0;
   if (m_str)
   {
      // Read Header
      Q_INT32 i,n;
      (*m_str) >> i;
      m_fastPatternOffset = i;
      (*m_str) >> i;
      m_otherPatternOffset = i;
      (*m_str) >> n;
      QString str;
      for(;n;n--)
      {
         (*m_str) >> str >> i;
         m_propertyTypeDict.insert(str, i);
      }
   }
}


KServiceTypeFactory::~KServiceTypeFactory()
{
  _self = 0L;
  KServiceTypeProfile::clear();
}

KServiceTypeFactory * KServiceTypeFactory::self()
{
  if (!_self)
    _self = new KServiceTypeFactory();
  return _self;
}

KServiceType * KServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
   if (!m_sycocaDict) return 0L; // Error!
   assert (!KSycoca::self()->isBuilding());
   int offset = m_sycocaDict->find_string( _name );
   if (!offset) return 0; // Not found
   KServiceType * newServiceType = createEntry(offset);

   // Check whether the dictionary was right.
   if (newServiceType && (newServiceType->name() != _name))
   {
     // No it wasn't...
     delete newServiceType;
     newServiceType = 0; // Not found
   }
   return newServiceType;
}

QVariant::Type KServiceTypeFactory::findPropertyTypeByName(const QString &_name)
{
   if (!m_sycocaDict) 
      return QVariant::Invalid; // Error!

   assert (!KSycoca::self()->isBuilding());

   QMapConstIterator<QString,int> it = m_propertyTypeDict.find(_name);
   if (it != m_propertyTypeDict.end()) {
     return (QVariant::Type)it.data();
   }

   return QVariant::Invalid;
}

KMimeType * KServiceTypeFactory::findFromPattern(const QString &_filename)
{
   // Assume we're NOT building a database
   if (!m_str) return 0;

   // Get stream to the header
   QDataStream *str = m_str;

   str->device()->at( m_fastPatternOffset );

   Q_INT32 nrOfEntries;
   (*str) >> nrOfEntries;
   Q_INT32 entrySize;
   (*str) >> entrySize;

   Q_INT32 fastOffset =  str->device()->at( );

   QString pattern;
   Q_INT32 mimetypeOffset;
   KServiceType * newServiceType = 0L;

   // Let's go for a binary search in the "fast" pattern index
   Q_INT32 left = 0;
   Q_INT32 right = nrOfEntries - 1;
   Q_INT32 middle;
   // Extract extension
   int lastDot = _filename.findRev('.');
   if (lastDot != -1) // if no '.', skip the extension lookup
   {
      QString extension = _filename.right( _filename.length() - _filename.findRev('.') - 1 );
      extension = extension.leftJustify(4);

      while (left <= right) {
         middle = (left + right) / 2;
         // read pattern at position "middle"
         str->device()->at( middle * entrySize + fastOffset );
         (*str) >> pattern;
         int cmp = pattern.compare( extension );
         if (cmp < 0)
            left = middle + 1;
         else if (cmp == 0) // found
         {
            assert( newServiceType == 0L );
            (*str) >> mimetypeOffset;
            newServiceType = createEntry(mimetypeOffset);
            assert (newServiceType && newServiceType->isType( KST_KMimeType ));
            // don't return newServiceType - there may be an "other" pattern that
            // matches best this file, like *.tar.bz
            break; // but get out of the fast patterns
         }
         else
            right = middle - 1;
      }
   }

   // Now try the "other" Pattern table
   str->device()->at( m_otherPatternOffset );

   while (true)
   {
      (*str) >> pattern;
      if (pattern.isEmpty()) // end of list
          return (KMimeType *) newServiceType; // return what we got (0L or real one)
      (*str) >> mimetypeOffset;
      if ( KStringHandler::matchFilename( _filename, pattern ) )
      {
         if (newServiceType) // we got one, but it's not good enough (like *.bz for a tar.bz file)
             delete newServiceType;
         newServiceType = createEntry(mimetypeOffset);
         assert (newServiceType && newServiceType->isType( KST_KMimeType ));
         return (KMimeType *) newServiceType;
      }
   }
}

KMimeType::List KServiceTypeFactory::allMimeTypes()
{
   KMimeType::List result;
   KSycocaEntry::List list = allEntries();
   for( KSycocaEntry::List::Iterator it = list.begin();
        it != list.end();
        ++it)
   {
      KMimeType *newMimeType = dynamic_cast<KMimeType *>((*it).data());
      if (newMimeType)
         result.append( KMimeType::Ptr( newMimeType ) );
   }
   return result;
}

KServiceType::List KServiceTypeFactory::allServiceTypes()
{
   KServiceType::List result;
   KSycocaEntry::List list = allEntries();
   for( KSycocaEntry::List::Iterator it = list.begin();
        it != list.end();
        ++it)
   {
#ifndef Q_WS_QWS
      KServiceType *newServiceType = dynamic_cast<KServiceType *>((*it).data());
#else //FIXME
      KServiceType *newServiceType = (KServiceType*)(*it).data();
#endif
      if (newServiceType)
         result.append( KServiceType::Ptr( newServiceType ) );
   }
   return result;
}

bool KServiceTypeFactory::checkMimeTypes()
{
   QDataStream *str = KSycoca::self()->findFactory( factoryId() );
   if (!str) return false;

   // check if there are mimetypes/servicetypes
   return (m_beginEntryOffset != m_endEntryOffset);
}

KServiceType * KServiceTypeFactory::createEntry(int offset)
{
   KServiceType *newEntry = 0;
   KSycocaType type;
   QDataStream *str = KSycoca::self()->findEntry(offset, type);
   if (!str) return 0;

   switch(type)
   {
     case KST_KServiceType:
        newEntry = new KServiceType(*str, offset);
        break;
     case KST_KMimeType:
        newEntry = new KMimeType(*str, offset);
        break;
     case KST_KFolderType:
        newEntry = new KFolderType(*str, offset);
        break;
     case KST_KDEDesktopMimeType:
        newEntry = new KDEDesktopMimeType(*str, offset);
        break;
     case KST_KExecMimeType:
        newEntry = new KExecMimeType(*str, offset);
        break;

     default:
        kdError(7011) << QString("KServiceTypeFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type) << endl;
        break;
   }
   if (!newEntry->isValid())
   {
      kdError(7011) << "KServiceTypeFactory: corrupt object in KSycoca database!\n" << endl;
      delete newEntry;
      newEntry = 0;
   }
   return newEntry;
}

KServiceTypeFactory *KServiceTypeFactory::_self = 0;
