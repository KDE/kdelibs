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

KSycocaFactory::KSycocaFactory(KSycocaFactoryId factory_id)
 : m_pathList(0), m_entryList(0)
{
  if (!KSycoca::isBuilding()) // read-only database ?
   {
      QDataStream *str = KSycoca::registerFactory( factory_id );
      // can't call factoryId() here since the constructor can't call inherited methods

      // Read position of index tables....
      Q_INT32 entryDictOffset;
      (*str) >> entryDictOffset;

      // Init index tables
      m_entryDict = new KSycocaDict(str, entryDictOffset);   
   }
   else
   {
      // Build new database!
      m_pathList = new QStringList();
      m_entryList = new KSycocaEntryList();
      m_entryList->setAutoDelete(true);
      m_entryDict = new KSycocaDict();

      // m_pathList will be filled in by inherited constructors
   }
}

KSycocaFactory::~KSycocaFactory()
{
   delete m_pathList;
   delete m_entryList;
   delete m_entryDict;
}

void KSycocaFactory::clear()
{
  m_entryDict->clear();
  m_entryList->clear();
}

void
KSycocaFactory::save(QDataStream &str)
{
   if (!m_entryList) return; // Error! Function should only be called when
                             // building database

   if (!m_entryDict) return; // Error!

   mOffset = str.device()->at(); // store position in member variable
   Q_INT32 entryDictOffset = 0;

   // Write header (pass #1)
   str << entryDictOffset;
   
   // Write all entries.
   for(KSycocaEntry *entry = m_entryList->first(); 
       entry; 
       entry=m_entryList->next())
   {
      entry->save(str);
   }

   // Write indices...
   entryDictOffset = str.device()->at();      

   m_entryDict->save(str);

   int endOfFactoryData = str.device()->at();

   // Update header (pass #2)
   str.device()->at(mOffset);
   str << entryDictOffset;

   // Seek to end.
   str.device()->at(endOfFactoryData);
}

void 
KSycocaFactory::addEntry(KSycocaEntry *newEntry)
{
   if (!m_entryList) return; // Error! Function should only be called when
                             // building database

   if (!m_entryDict) return; // Error!

   m_entryList->append(newEntry);
   m_entryDict->add(newEntry->name(), newEntry);
}
