/*
   This file is part of the KDE libraries
   Copyright (c) 1999-2000 Preston Brown <pbrown@kde.org>
   Copyright (C) 1996-2000 Matthias Kalle Dalheimer <kalle@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef _KCONFIGDATA_H
#define _KCONFIGDATA_H

#include <qmap.h> // generic red-black tree class

/**
 * map/dict/list node entry.
 * @internal
 */
struct KEntry
{
  KEntry()
    : mValue(0), bDirty(false), bNLS(false), 
      bGlobal(false), bImmutable(false), bDeleted(false) {}
  QCString mValue;
  bool    bDirty :1;  // must the entry be written back to disk?
  bool    bNLS   :1;  // entry should be written with locale tag
  bool    bGlobal:1;  // entry should be written to the global config file
  bool    bImmutable:1; // Entry can not be modified
  bool    bDeleted:1; // Entry has been deleted
};

/**
 * key structure holding both the actual key and the the group
 * to which it belongs.
 * @internal
 */
struct KEntryKey
{
  KEntryKey(const QCString& _group = 0,
	    const QCString& _key = 0)
      : mGroup(_group), mKey(_key), bLocal(false), bDefault(false),
        c_key(_key.data()) {}
  QCString mGroup; // the "group" to which this EntryKey belongs
  QCString mKey;   // the _actual_ key of the entry in question
  bool    bLocal  :1; // Entry is localised
  bool    bDefault:1; // Entry indicates default value
  
  const char *c_key;
};

/**
 * compares two KEntryKeys (needed for QMap).
 * @internal
 */
inline bool operator <(const KEntryKey &k1, const KEntryKey &k2)
{
   //saves one strcmp on each call
   int result=qstrcmp(k1.mGroup.data(),k2.mGroup.data());
   if (result!=0)
      return (result<0);     

  if (!k1.c_key && k2.c_key)
    return true;

  result = 0;
  if (k1.c_key && k2.c_key)
     result = strcmp(k1.c_key, k2.c_key);
  if (result != 0)
     return result < 0;
  if (!k1.bLocal && k2.bLocal)
    return true;
  if (k1.bLocal && !k2.bLocal)
    return false;
  return (!k1.bDefault && k2.bDefault);
}

/**
 * type specifying a map of entries (key,value pairs).
 * The keys are actually a key in a particular config file group together
 * with the group name.
 * @internal
 */
typedef QMap<KEntryKey, KEntry> KEntryMap;

/**
 * type for iterating over keys in a KEntryMap in sorted order.
 * @internal
 */
typedef QMap<KEntryKey, KEntry>::Iterator KEntryMapIterator;

/**
 * type for iterating over keys in a KEntryMap in sorted order.
 * It is const, thus you cannot change the entries in the iterator,
 * only examine them.
 * @internal
 */
typedef QMap<KEntryKey, KEntry>::ConstIterator KEntryMapConstIterator;

#endif
