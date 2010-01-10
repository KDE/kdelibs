/*  This file is part of the KDE libraries
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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

#include "kbuildservicegroupfactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"
#include <kservicegroup_p.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>
#include <QtCore/QHash>

KBuildServiceGroupFactory::KBuildServiceGroupFactory() :
  KServiceGroupFactory()
{
   m_resourceList = new KSycocaResourceList;
//   m_resourceList->add( "apps", "*.directory" );

   m_baseGroupDict = new KSycocaDict();
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildServiceGroupFactory::resourceTypes()
{
    return QStringList(); // << "apps";
}

KBuildServiceGroupFactory::~KBuildServiceGroupFactory()
{
   delete m_resourceList;
}

KServiceGroup *
KBuildServiceGroupFactory::createEntry( const QString&, const char * ) const
{
  // Unused
  kWarning(7021) << "called!";
  return 0;
}


void KBuildServiceGroupFactory::addNewEntryTo( const QString &menuName, const KService::Ptr& newEntry)
{
  KSycocaEntry::Ptr ptr = m_entryDict->value(menuName);
  KServiceGroup::Ptr entry;
  if (ptr && ptr->isType(KST_KServiceGroup))
    entry = KServiceGroup::Ptr::staticCast( ptr );

  if (!entry)
  {
    kWarning(7021) << "( " << menuName << ", " << newEntry->name() << " ): menu does not exists!";
    return;
  }
  entry->addEntry( KSycocaEntry::Ptr::staticCast( newEntry ) );
}

KServiceGroup::Ptr
KBuildServiceGroupFactory::addNew( const QString &menuName, const QString& file, KServiceGroup::Ptr entry, bool isDeleted)
{
  KSycocaEntry::Ptr ptr = m_entryDict->value(menuName);
  if (ptr)
  {
    kWarning(7021) << "( " << menuName << ", " << file << " ): menu already exists!";
    return KServiceGroup::Ptr::staticCast( ptr );
  }

  // Create new group entry
  if (!entry)
     entry = new KServiceGroup(file, menuName);

  entry->d_func()->m_childCount = -1; // Recalculate

  addEntry( KSycocaEntry::Ptr::staticCast(entry) );

  if (menuName != "/")
  {
     // Make sure parent dir exists.
     QString parent = menuName.left(menuName.length()-1);
     int i = parent.lastIndexOf('/');
     if (i > 0) {
        parent = parent.left(i+1);
     } else {
        parent = '/';
     }


     KServiceGroup::Ptr parentEntry;
     ptr = m_entryDict->value(parent);
     if (ptr && ptr->isType(KST_KServiceGroup))
         parentEntry = KServiceGroup::Ptr::staticCast( ptr );
     if (!parentEntry)
     {
        kWarning(7021) << "( " << menuName << ", " << file << " ): parent menu does not exist!";
     }
     else
     {
        if (!isDeleted && !entry->isDeleted())
           parentEntry->addEntry( KSycocaEntry::Ptr::staticCast( entry ) );
     }
  }
  return entry;
}

void
KBuildServiceGroupFactory::addNewChild( const QString &parent, const KSycocaEntry::Ptr& newEntry)
{
  QString name = "#parent#"+parent;

  KServiceGroup::Ptr entry;
  KSycocaEntry::Ptr ptr = m_entryDict->value(name);
  if (ptr && ptr->isType(KST_KServiceGroup))
     entry = KServiceGroup::Ptr::staticCast( ptr );

  if (!entry)
  {
     entry = new KServiceGroup(name);
     addEntry( KSycocaEntry::Ptr::staticCast( entry ) );
  }
  if (newEntry)
     entry->addEntry( newEntry );
}

void
KBuildServiceGroupFactory::addEntry( const KSycocaEntry::Ptr& newEntry)
{
   KSycocaFactory::addEntry(newEntry);

   KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast( newEntry );
   serviceGroup->d_func()->m_serviceList.clear();

   if ( !serviceGroup->baseGroupName().isEmpty() )
   {
       m_baseGroupDict->add( serviceGroup->baseGroupName(), newEntry );
   }
}

void
KBuildServiceGroupFactory::saveHeader(QDataStream &str)
{
   KSycocaFactory::saveHeader(str);

   str << (qint32) m_baseGroupDictOffset;
}

void
KBuildServiceGroupFactory::save(QDataStream &str)
{
   KSycocaFactory::save(str);

   m_baseGroupDictOffset = str.device()->pos();
   m_baseGroupDict->save(str);

   int endOfFactoryData = str.device()->pos();

   // Update header (pass #3)
   saveHeader(str);

   // Seek to end.
   str.device()->seek(endOfFactoryData);
}

KServiceGroup::Ptr KBuildServiceGroupFactory::findGroupByDesktopPath( const QString &_name, bool deep )
{
    assert (KSycoca::self()->isBuilding());
    Q_UNUSED(deep); // ?
    // We're building a database - the service type must be in memory
    KSycocaEntry::Ptr group = m_entryDict->value( _name );
    return KServiceGroup::Ptr::staticCast( group );
}
