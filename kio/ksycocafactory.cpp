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

#include "ksycoca.h"
#include "ksycocatype.h"
#include "ksycocafactory.h"
#include "ksycocaentry.h"
#include "ksycocadict.h"
#include <qstringlist.h>
#include <qdict.h>
#include <kdebug.h>

KSycocaFactory::KSycocaFactory(KSycocaFactoryId factory_id)
 : m_pathList(0), m_entryDict(0), m_sycocaDict(0)
{
  if (!KSycoca::self()->isBuilding()) // read-only database ?
   {
      m_str = KSycoca::self()->findFactory( factory_id );
      // can't call factoryId() here since the constructor can't call inherited methods
      if (!m_str) return; // No database!

      // Read position of index tables....
      Q_INT32 i;
      (*m_str) >> i;
      m_sycocaDictOffset = i;
      (*m_str) >> i;
      m_beginEntryOffset = i;
      (*m_str) >> i;
      m_endEntryOffset = i;

      int saveOffset = m_str->device()->at();
      // Init index tables
      m_sycocaDict = new KSycocaDict(m_str, m_sycocaDictOffset);   
      saveOffset = m_str->device()->at(saveOffset);
   }
   else
   {
      // Build new database!
      m_str = 0;
      m_pathList = new QStringList();
      m_entryDict = new KSycocaEntryDict();
      m_entryDict->setAutoDelete(true);
      m_sycocaDict = new KSycocaDict();
      m_beginEntryOffset = 0;
      m_endEntryOffset = 0;

      // m_pathList will be filled in by inherited constructors
   }
   KSycoca::self()->addFactory(this);
}

KSycocaFactory::~KSycocaFactory()
{
   delete m_pathList;
   delete m_entryDict;
   delete m_sycocaDict;
}

void KSycocaFactory::clear()
{
  if (!m_sycocaDict) return; // Error!
  m_sycocaDict->clear();
  m_entryDict->clear();
}

void
KSycocaFactory::saveHeader(QDataStream &str)
{
   // Write header 
   str.device()->at(mOffset);
   str << (Q_INT32) m_sycocaDictOffset;
   str << (Q_INT32) m_beginEntryOffset;
   str << (Q_INT32) m_endEntryOffset;
}

void
KSycocaFactory::save(QDataStream &str)
{
   if (!m_entryDict) return; // Error! Function should only be called when
                             // building database
   if (!m_sycocaDict) return; // Error!

   mOffset = str.device()->at(); // store position in member variable
   m_sycocaDictOffset = 0;

   // Write header (pass #1)
   saveHeader(str);

   m_beginEntryOffset = str.device()->at();

   // Write all entries.
   for(QDictIterator<KSycocaEntry> it ( *m_entryDict ); 
       it.current(); 
       ++it)
   {
      if ( !it.current()->isDeleted() )
         it.current()->save(str);
   }

   m_endEntryOffset = str.device()->at();

   // Write indices...
   m_sycocaDictOffset = str.device()->at();      

   m_sycocaDict->save(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #2)
   saveHeader(str);

   // Seek to end.
   str.device()->at(endOfFactoryData);
}

void 
KSycocaFactory::addEntry(KSycocaEntry *newEntry)
{
   if (!m_entryDict) return; // Error! Function should only be called when
                             // building database

   if (!m_sycocaDict) return; // Error!

   QString name = newEntry->name();
   //kdebug( KDEBUG_INFO, 7011, QString("SycocaFactory : adding entry %1").arg(name) );
   KSycocaEntry * oldEntry = (*m_entryDict)[ name ];
   // If there is any previous entry with the same name (e.g. local .desktop file)
   // don't do anything
   // This is because local dirs are parsed BEFORE global dirs.
   if ( oldEntry )
   {
     //kdebug( KDEBUG_INFO, 7011, QString("SycocaFactory : keeping old entry, and deleting new one") );
     delete newEntry;
   }
   else
   {
     m_entryDict->insert( name, newEntry );
     m_sycocaDict->add( name, newEntry );
   }
}
