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

#include <qdict.h>
#include <qlist.h>
#include <qstring.h>
class KSycoca;
class QStringList;
class KSycocaDict;
class KSycocaEntry;

typedef QDict<KSycocaEntry> KSycocaEntryDict;

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
   virtual KSycocaEntry *createEntry(const QString &file) = 0L;

   /**
    * Add an entry
    */
   void addEntry(KSycocaEntry *newEntry);
   /**
    * Clear the whole factory - often called before updating it
    * Destroys every entry since the list and the dict are autodelete
    * Keeps the paths.
    */
   void clear();

   /**
    * Saves all entries it maintains as well as index files
    * for these entries to the stream 'str'.
    * Also sets mOffset to the starting position.
    */
   virtual void save(QDataStream &str);

   /**
    * @return the path for which this factory is responsible.
    *         Please note that the return value may not have a trailing '/'.
    */
   virtual const QStringList * pathList() const { return m_pathList; }

private:
   int mOffset;

protected:
   QStringList *m_pathList;
   KSycocaEntryDict *m_entryDict;
   KSycocaDict *m_sycocaDict;
};

// This, instead of a typedef, allows to declare "class ..." in header files
class KSycocaFactoryList : public QList<KSycocaFactory>
{
public:
   KSycocaFactoryList() { }
};

#endif
