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

template class QDict<KSycocaEntry>;
template class QDict<KSharedPtr<KSycocaEntry> >;

KSycocaFactory::KSycocaFactory(KSycocaFactoryId factory_id)
 : m_resourceList(0), m_entryDict(0), m_sycocaDict(0)
{
  if (!KSycoca::self()->isBuilding()) // read-only database ?
  {
      m_str = KSycoca::self()->findFactory( factory_id );
      // can't call factoryId() here since the constructor can't call inherited methods
      if (m_str) // Can be 0 in case of errors
      {
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
   }
   else
   {
      // Build new database!
      m_str = 0;
      m_resourceList = 0;
      m_entryDict = new KSycocaEntryDict(977);
      m_entryDict->setAutoDelete(true);
      m_sycocaDict = new KSycocaDict();
      m_beginEntryOffset = 0;
      m_endEntryOffset = 0;

      // m_resourceList will be filled in by inherited constructors
   }
   KSycoca::self()->addFactory(this);
}

KSycocaFactory::~KSycocaFactory()
{
   delete m_entryDict;
   delete m_sycocaDict;
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
   int entryCount = 0;
   for(QDictIterator<KSycocaEntry::Ptr> it ( *m_entryDict ); 
       it.current(); 
       ++it)
   {
      KSycocaEntry *entry = (*it.current());
      entry->save(str);
      entryCount++;
   }

   m_endEntryOffset = str.device()->at();

   // Write indices...
   // Linear index
   str << (Q_INT32) entryCount;
   for(QDictIterator<KSycocaEntry::Ptr> it ( *m_entryDict ); 
       it.current(); 
       ++it)
   {
      KSycocaEntry *entry = (*it.current());
      str << (Q_INT32) entry->offset(); 
   }

   // Dictionary index
   m_sycocaDictOffset = str.device()->at();      
   m_sycocaDict->save(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #2)
   saveHeader(str);

   // Seek to end.
   str.device()->at(endOfFactoryData);
}

void 
KSycocaFactory::addEntry(KSycocaEntry *newEntry, const char *)
{
   if (!m_entryDict) return; // Error! Function should only be called when
                             // building database

   if (!m_sycocaDict) return; // Error!

   QString name = newEntry->name();
   m_entryDict->insert( name, new KSycocaEntry::Ptr(newEntry) );
   m_sycocaDict->add( name, newEntry );
}

KSycocaEntry::List KSycocaFactory::allEntries()
{
   KSycocaEntry::List list;
   if (!m_str) return list;

   // Assume we're NOT building a database

   m_str->device()->at(m_endEntryOffset);
   Q_INT32 entryCount;
   (*m_str) >> entryCount;

   Q_INT32 *offsetList = new Q_INT32[entryCount];
   for(int i = 0; i < entryCount; i++)
   {
      (*m_str) >> offsetList[i];
   }

   for(int i = 0; i < entryCount; i++)
   {
      KSycocaEntry *newEntry = createEntry(offsetList[i]);
      if (newEntry)
      {
         list.append( KSycocaEntry::Ptr( newEntry ) );
      }
   }
   delete [] offsetList;
   return list;
}

void KSycocaFactory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

