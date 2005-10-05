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

#ifndef __ksycocadict_h__
#define __ksycocadict_h__

#include <qstring.h>
#include <q3valuelist.h>
#include <qdatastream.h>
#include "kdelibs_export.h"

class KSycocaEntry;
class KSycocaDictStringList;

/**
 * @internal
 * Hash table implementation for the sycoca database file
 */
class KDECORE_EXPORT KSycocaDict
{
public:
   /**
    * Create an empty dict, for building the database
    */
   KSycocaDict();
   /**
    * Create a dict from an existing database
    */
   KSycocaDict(QDataStream *str, int offset);

   ~KSycocaDict();

   /**
    * Adds a 'payload' to the dictionary with key 'key'.
    * 
    * 'payload' should have a valid offset by the time  
    * the dictionary gets saved.
    **/
   void add(const QString &key, KSycocaEntry *payload);

   /**
    * Removes the 'payload' from the dictionary with key 'key'.
    * 
    * Not very fast, use with care O(N)
    **/
   void remove(const QString &key);
   
   /**
    * Looks up an entry identified by 'key'.
    *
    * If 0 is returned, no matching entry exists.
    * Otherwise, the offset of the entry is returned. 
    *
    * NOTE: It is not guaranteed that this entry is
    * indeed the one you were looking for.
    * After loading the entry you should check that it
    * indeed matches the search key. If it doesn't
    * then no matching entry exists.
    */
   int find_string(const QString &key );
   
   /**
    * The number of entries in the dictionary.
    *
    * Only valid when building the database.
    */
   uint count();
   
   /**
    * Reset the dictionary.
    *
    * Only valid when building the database.
    */
   void clear();
   
   /**
    * Save the dictionary to the stream
    * A reasonable fast hash algorithm will be created.
    *
    * Typically this will find 90% of the entries directly.
    * Average hash table size: nrOfItems * 20 bytes.
    * Average duplicate list size: nrOfItms * avgKeyLength / 5.
    *
    * Unknown keys have an average 20% chance to give a false hit.
    * (That's why your program should check the result)
    *
    * Example:
    *   Assume 1000 items with an average key length of 60 bytes.
    *
    *   Approx. 900 items will hash directly to the right entry.
    *   Approx. 100 items require a lookup in the duplicate list.
    *
    *   The hash table size will be approx. 20Kb.
    *   The duplicate list size will be approx. 12Kb.
    **/    
   void save(QDataStream &str);

protected:
   Q_UINT32 hashKey( const QString &);
private:
   KSycocaDictStringList *d;
   QDataStream *mStr;
   Q_INT32 mOffset;
   Q_UINT32 mHashTableSize;
   QList<Q_INT32> mHashList;
};

#endif
