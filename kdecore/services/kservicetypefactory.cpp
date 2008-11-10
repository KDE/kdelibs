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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kservicetypefactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocadict.h"
#include "kservicetypeprofile.h"

#include <kdebug.h>
#include <assert.h>

KServiceTypeFactory::KServiceTypeFactory()
 : KSycocaFactory( KST_KServiceTypeFactory )
{
   _self = this;
   if (m_str)
   {
      // Read Header
      qint32 n;
      (*m_str) >> n;
      if (n > 1024)
      {
         KSycoca::flagError();
      }
      else
      {
         QString str;
         qint32 i;
         for(;n;--n)
         {
            KSycocaEntry::read(*m_str, str);
            (*m_str) >> i;
            m_propertyTypeDict.insert(str, i);
         }
      }
   }
}


KServiceTypeFactory::~KServiceTypeFactory()
{
  _self = 0;
  KServiceTypeProfile::clearCache();
}

KServiceTypeFactory * KServiceTypeFactory::self()
{
  if (!_self)
    _self = new KServiceTypeFactory();
  return _self;
}

KServiceType::Ptr KServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
   if (!sycocaDict()) return KServiceType::Ptr(); // Error!
   assert (!KSycoca::self()->isBuilding());
   int offset = sycocaDict()->find_string( _name );
   if (!offset) return KServiceType::Ptr(); // Not found
   KServiceType::Ptr newServiceType(createEntry(offset));

   // Check whether the dictionary was right.
   if (newServiceType && (newServiceType->name() != _name))
   {
     // No it wasn't...
     newServiceType = 0; // Not found
   }
   return newServiceType;
}

QVariant::Type KServiceTypeFactory::findPropertyTypeByName(const QString &_name)
{
   if (!sycocaDict())
      return QVariant::Invalid; // Error!

   assert (!KSycoca::self()->isBuilding());

   return static_cast<QVariant::Type>( m_propertyTypeDict.value( _name, QVariant::Invalid ) );
}

KServiceType::List KServiceTypeFactory::allServiceTypes()
{
   KServiceType::List result;
   const KSycocaEntry::List list = allEntries();
   for( KSycocaEntry::List::ConstIterator it = list.begin();
        it != list.end();
        ++it)
   {
      if ( (*it)->isType( KST_KServiceType ) ) {
          KServiceType::Ptr newServiceType = KServiceType::Ptr::staticCast( *it );
          result.append( newServiceType );
      }
   }
   return result;
}

KServiceType * KServiceTypeFactory::createEntry(int offset) const
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
     default:
        kError(7011) << QString("KServiceTypeFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type) << endl;
        break;
   }
   if (newEntry && !newEntry->isValid())
   {
      kError(7011) << "KServiceTypeFactory: corrupt object in KSycoca database!\n" << endl;
      delete newEntry;
      newEntry = 0;
   }
   return newEntry;
}

KServiceTypeFactory *KServiceTypeFactory::_self = 0;

void KServiceTypeFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }

// vim: ts=3 sw=3 et
