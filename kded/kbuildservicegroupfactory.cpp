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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kbuildservicegroupfactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"
#include "kresourcelist.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>

KBuildServiceGroupFactory::KBuildServiceGroupFactory() :
  KServiceGroupFactory()
{
   m_resourceList = new KSycocaResourceList();
   m_resourceList->add( "apps", "*.directory" );
}

// return all service types for this factory
// i.e. first arguments to m_resourceList->add() above
QStringList KBuildServiceGroupFactory::resourceTypes()
{
    return QStringList() << "apps";
}

KBuildServiceGroupFactory::~KBuildServiceGroupFactory()
{
   delete m_resourceList;
}

KServiceGroup *
KBuildServiceGroupFactory::createEntry( const QString& file, const char *resource )
{
  return addNewEntry(file, resource, 0);
}

KServiceGroup *
KBuildServiceGroupFactory::addNewEntry( const QString& file, const char *resource, KSycocaEntry *newEntry)
{
  if (strcmp(resource, "apps") != 0) return 0;

  QString name = file;
  int pos = name.findRev('/');
  if (pos != -1) {
     name = name.left(pos+1);
  } else {
     name = "/";
  }

  KServiceGroup *entry = 0;
  KSycocaEntry::Ptr *ptr = m_entryDict->find(name);
  if (ptr)
     entry = dynamic_cast<KServiceGroup *>(ptr->data());

  if (!entry)
  {
     // Create new group entry
     QString fullPath = locate( resource, name + ".directory");

     entry = new KServiceGroup(fullPath, name);
     addEntry( entry, resource );

     if (name != "/")
     {
        // Make sure parent dir exists.
        KServiceGroup *parentEntry = 0;
        QString parent = name.left(name.length()-1);
        int i = parent.findRev('/');
        if (i > 0) {
           parent = parent.left(i+1);
        } else {
           parent = "/";
        }
        parentEntry = 0;
        ptr = m_entryDict->find(parent);
        if (ptr)
           parentEntry = dynamic_cast<KServiceGroup *>(ptr->data());
        if (!parentEntry)
        {
           parentEntry = addNewEntry( parent, resource, 0 );
        }
        if (parentEntry && !entry->isDeleted())
           parentEntry->addEntry( entry );
     }
  }
  if (newEntry)
     entry->addEntry( newEntry );

  return entry;
}

KServiceGroup *
KBuildServiceGroupFactory::addNewChild( const QString &parent, const char *resource, KSycocaEntry *newEntry)
{
  QString name = "#parent#"+parent;

  KServiceGroup *entry = 0;
  KSycocaEntry::Ptr *ptr = m_entryDict->find(name);
  if (ptr)
     entry = dynamic_cast<KServiceGroup *>(ptr->data());

  if (!entry)
  {
     entry = new KServiceGroup(name);
     addEntry( entry, resource );
  }
  if (newEntry)
     entry->addEntry( newEntry );

  return entry;

}

void
KBuildServiceGroupFactory::addEntry( KSycocaEntry *newEntry, const char *resource)
{
   KSycocaFactory::addEntry(newEntry, resource);
   KServiceGroup * serviceGroup = (KServiceGroup *) newEntry;

   if ( !serviceGroup->baseGroupName().isEmpty() )
   {
       m_baseGroupDict->add( serviceGroup->baseGroupName(), newEntry );
   }
}

void
KBuildServiceGroupFactory::saveHeader(QDataStream &str)
{
   KSycocaFactory::saveHeader(str);

   str << (Q_INT32) m_baseGroupDictOffset;
}

void
KBuildServiceGroupFactory::save(QDataStream &str)
{
   KSycocaFactory::save(str);

   m_baseGroupDictOffset = str.device()->at();
   m_baseGroupDict->save(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #3)
   saveHeader(str);

   // Seek to end.
   str.device()->at(endOfFactoryData);
}
