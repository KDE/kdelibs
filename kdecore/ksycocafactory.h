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

#ifndef __ksycocafactory_h__
#define __ksycocafactory_h__ "$Id$"

#include "ksycocatype.h"
#include "ksycocaentry.h"

#include <qdict.h>
#include <qptrlist.h>
class KSycoca;
class QStringList;
class QString;
class KSycocaDict;
class KSycocaResourceList;

typedef QDict<KSycocaEntry::Ptr> KSycocaEntryDict;

/**
 * @internal
 * Base class for sycoca factories
 */
class KSycocaFactory
{
public:
   virtual KSycocaFactoryId factoryId() const = 0;
 
protected: // virtual class
   /**
    * Create a factory which can be used to lookup from/create a database
    * (depending on KSycoca::isBuilding())
    */
   KSycocaFactory( KSycocaFactoryId factory_id );

public:
   virtual ~KSycocaFactory();

   /**
    * @return the position of the factory in the sycoca file
    */
   int offset() { return mOffset; }

   /**
    * @return the dict, for special use by KBuildSycoca
    */
   KSycocaEntryDict * entryDict() { return m_entryDict; }

   /**
    * Construct an entry from a config file.
    * To be implemented in the real factories.
    */
   virtual KSycocaEntry *createEntry(const QString &file, const char *resource) = 0;

   /**
    * Add an entry
    */
   virtual void addEntry(KSycocaEntry *newEntry, const char *resource);

   /**
    * Read an entry from the database
    */
   virtual KSycocaEntry *createEntry(int offset)=0;

   /**
    * Get a list of all entries from the database.
    */
   KSycocaEntry::List allEntries();

   /**
    * Saves all entries it maintains as well as index files
    * for these entries to the stream 'str'.
    *
    * Also sets mOffset to the starting position.
    *
    * The stream is positioned at the end of the last index.
    *
    * Don't forget to call the parent first when you override 
    * this function.
    */
   virtual void save(QDataStream &str);

   /**
    * Writes out a header to the stream 'str'.
    * The baseclass positions the stream correctly.
    *
    * Don't forget to call the parent first when you override 
    * this function.
    */
   virtual void saveHeader(QDataStream &str);

   /**
    * @return the resources for which this factory is responsible.
    */
   virtual const KSycocaResourceList * resourceList() const { return m_resourceList; }

private:
   int mOffset;

protected:
   int m_sycocaDictOffset;
   int m_beginEntryOffset;
   int m_endEntryOffset;
   QDataStream *m_str;

   KSycocaResourceList *m_resourceList;
   KSycocaEntryDict *m_entryDict;
   KSycocaDict *m_sycocaDict;
protected:
   virtual void virtual_hook( int id, void* data );
};

/** This, instead of a typedef, allows to declare "class ..." in header files
 * @internal
 */
class KSycocaFactoryList : public QPtrList<KSycocaFactory>
{
public:
   KSycocaFactoryList() { }
};

#endif
