// (C) 1996-1999 by Matthias Kalle Dalheimer
// (c) 1999 by Preston Brown <pbrown@kde.org>

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
    : aValue(QString::null), bDirty(false), bNLS(false), bGlobal(false) {}
  QString aValue;
  bool    bDirty;  // must the entry be written back to disk?
  bool    bNLS;    // entry should be written with locale tag
  bool    bGlobal; // entry should be written to the global config file
};

/**
 * key structure holding both the actual key and the the group
 * to which it belongs.
 * @internal
 */
struct KEntryKey
{
  QString group; // the "group" to which this EntryKey belongs
  QString key;   // the _actual_ key of the entry in question
};

/**
 * compares two KEntryKeys (needed for QMap).
 * @internal
 */
inline bool operator <(const KEntryKey &k1, const KEntryKey &k2) 
{ 
  if (k1.group != k2.group)
    return k1.group < k2.group;
  return k1.key < k2.key;
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
