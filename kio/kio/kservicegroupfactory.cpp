/*  This file is part of the KDE libraries
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
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

#include "kservicegroupfactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocadict.h"
#include "kservice.h"

#include <qstring.h>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

KServiceGroupFactory::KServiceGroupFactory()
 : KSycocaFactory( KST_KServiceGroupFactory )
{
   m_baseGroupDictOffset = 0;
   if (m_str)
   {
      // Read Header
      Q_INT32 i;
      (*m_str) >> i;
      m_baseGroupDictOffset = i;

      int saveOffset = m_str->device()->at();
      // Init index tables
      m_baseGroupDict = new KSycocaDict(m_str, m_baseGroupDictOffset);
      m_str->device()->at(saveOffset);
   }
   else
   {
      // Build new database
      m_baseGroupDict = new KSycocaDict();
   }
   _self = this;
}

KServiceGroupFactory::~KServiceGroupFactory()
{
   _self = 0L;
   delete m_baseGroupDict;
}

KServiceGroupFactory * KServiceGroupFactory::self()
{
  if (!_self)
    _self = new KServiceGroupFactory();
  return _self;
}

KServiceGroup * KServiceGroupFactory::findGroupByDesktopPath(const QString &_name, bool deep)
{
   if (!m_sycocaDict) return 0; // Error!

   // Warning : this assumes we're NOT building a database
   // But since findServiceByName isn't called in that case...
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_sycocaDict->find_string( _name );
   if (!offset) return 0; // Not found

   KServiceGroup * newGroup = createGroup(offset, deep);

   // Check whether the dictionary was right.
   if (newGroup && (newGroup->relPath() != _name))
   {
      // No it wasn't...
      delete newGroup;
      newGroup = 0; // Not found
   }
   return newGroup;
}

KServiceGroup * KServiceGroupFactory::findBaseGroup(const QString &_baseGroupName, bool deep)
{
   if (!m_baseGroupDict) return 0; // Error!

   // Warning : this assumes we're NOT building a database
   // But since findServiceByName isn't called in that case...
   // [ see KServiceTypeFactory for how to do it if needed ]

   int offset = m_baseGroupDict->find_string( _baseGroupName );
   if (!offset) return 0; // Not found

   KServiceGroup * newGroup = createGroup(offset, deep);

   // Check whether the dictionary was right.
   if (newGroup && (newGroup->baseGroupName() != _baseGroupName))
   {
      // No it wasn't...
      delete newGroup;
      newGroup = 0; // Not found
   }
   return newGroup;
}

KServiceGroup* KServiceGroupFactory::createGroup(int offset, bool deep)
{
   KServiceGroup * newEntry = 0L;
   KSycocaType type;
   QDataStream *str = KSycoca::self()->findEntry(offset, type);
   switch(type)
   {
     case KST_KServiceGroup:
        newEntry = new KServiceGroup(*str, offset, deep);
        break;

     default:
        kdError(7011) << QString("KServiceGroupFactory: unexpected object entry in KSycoca database (type = %1)").arg((int)type) << endl;
        return 0;
   }
   if (!newEntry->isValid())
   {
      kdError(7011) << "KServiceGroupFactory: corrupt object in KSycoca database!\n" << endl;
      delete newEntry;
      newEntry = 0;
   }
   return newEntry;
}

KServiceGroup* KServiceGroupFactory::createEntry(int offset)
{
   return createGroup(offset, true);
}

KServiceGroupFactory *KServiceGroupFactory::_self = 0;

void KServiceGroupFactory::virtual_hook( int id, void* data )
{ KSycocaFactory::virtual_hook( id, data ); }

