/*
   This file is part of the KDE libraries
   Copyright (c) 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCONFIGDATA_H
#define KCONFIGDATA_H

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QDebug>

/**
 * map/dict/list config node entry.
 * @internal
 */
struct KEntry
{
  /** Constructor. @internal */
  KEntry()
    : mValue(), bDirty(false),
      bGlobal(false), bImmutable(false), bDeleted(false), bExpand(false) {}
  /** @internal */
  QByteArray mValue;
  /**
   * Must the entry be written back to disk?
   */
  bool    bDirty :1;
  /**
   * Entry should be written to the global config file
   */
  bool    bGlobal:1;
  /**
   * Entry can not be modified.
   */
  bool    bImmutable:1;
  /**
   * Entry has been deleted.
   */
  bool    bDeleted:1;
  /**
   * Whether to apply dollar expansion or not.
   */
  bool    bExpand:1;
};

// These operators are used to check whether an entry which is about
// to be written equals the previous value. As such, this intentionally
// omits the dirty flag from the comparison.
inline bool operator ==(const KEntry &k1, const KEntry &k2)
{
    return k1.bGlobal == k2.bGlobal && k1.bImmutable == k2.bImmutable
           && k1.bDeleted == k2.bDeleted && k1.bExpand == k2.bExpand
           && k1.mValue == k2.mValue;
}

inline bool operator !=(const KEntry &k1, const KEntry &k2)
{
    return !(k1 == k2);
}

/**
 * key structure holding both the actual key and the group
 * to which it belongs.
 * @internal
 */
struct KEntryKey
{
  /** Constructor. @internal */
  KEntryKey(const QByteArray& _group = QByteArray(),
	    const QByteArray& _key = QByteArray(), bool isLocalized=false, bool isDefault=false)
      : mGroup(_group), mKey(_key), bLocal(isLocalized), bDefault(isDefault), bRaw(false)
      { ; }
  /**
   * The "group" to which this EntryKey belongs
   */
  QByteArray mGroup;
  /**
   * The _actual_ key of the entry in question
   */
  QByteArray mKey;
  /**
   * Entry is localised or not
   */
  bool    bLocal  :1;
  /**
   * Entry indicates if this is a default value.
   */
  bool    bDefault:1;
  /** @internal
   * Key is a raw unprocessed key.
   * @warning this should only be set during merging, never for normal use.
   */
  bool    bRaw:1;
};

/**
 * Compares two KEntryKeys (needed for QMap). The order is localized, localized-default,
 * non-localized, non-localized-default
 * @internal
 */
inline bool operator <(const KEntryKey &k1, const KEntryKey &k2)
{
    int result = qstrcmp(k1.mGroup, k2.mGroup);
    if (result != 0) {
        return result < 0;
    }

    result = qstrcmp(k1.mKey, k2.mKey);
    if (result != 0) {
        return result < 0;
    }

    if (k1.bLocal != k2.bLocal)
        return k1.bLocal;
    return (!k1.bDefault && k2.bDefault);
}


QDebug operator<<(QDebug dbg, const KEntryKey& key);
QDebug operator<<(QDebug dbg, const KEntry& entry);

/**
 * \relates KEntry
 * type specifying a map of entries (key,value pairs).
 * The keys are actually a key in a particular config file group together
 * with the group name.
 * @internal
 */
class KEntryMap : public QMap<KEntryKey, KEntry>
{
    public:
        enum SearchFlag {
            SearchDefaults=1,
            SearchLocalized=2
        };
        Q_DECLARE_FLAGS(SearchFlags, SearchFlag)

        enum EntryOption {
            EntryDirty=1,
            EntryGlobal=2,
            EntryImmutable=4,
            EntryDeleted=8,
            EntryExpansion=16,
            EntryRawKey=32,
            EntryDefault=(SearchDefaults<<16),
            EntryLocalized=(SearchLocalized<<16)
        };
        Q_DECLARE_FLAGS(EntryOptions, EntryOption)

        Iterator findExactEntry(const QByteArray& group, const QByteArray& key = QByteArray(),
                           SearchFlags flags = SearchFlags());

        Iterator findEntry(const QByteArray& group, const QByteArray& key = QByteArray(),
                           SearchFlags flags = SearchFlags());

        ConstIterator findEntry(const QByteArray& group, const QByteArray& key = QByteArray(),
                                SearchFlags flags = SearchFlags()) const;

        /**
         * Returns true if the entry gets dirtied or false in other case
         */
        bool setEntry(const QByteArray& group, const QByteArray& key,
                      const QByteArray& value, EntryOptions options);

        void setEntry(const QByteArray& group, const QByteArray& key,
                      const QString & value, EntryOptions options)
        {
            setEntry(group, key, value.toUtf8(), options);
        }

        QString getEntry(const QByteArray& group, const QByteArray& key,
                         const QString & defaultValue = QString(),
                         SearchFlags flags = SearchFlags(),
                         bool * expand=0) const;

        bool hasEntry(const QByteArray& group, const QByteArray& key = QByteArray(),
                      SearchFlags flags = SearchFlags()) const;

        bool getEntryOption(const ConstIterator& it, EntryOption option) const;
        bool getEntryOption(const QByteArray& group, const QByteArray& key,
                            SearchFlags flags, EntryOption option) const
        {
            return getEntryOption(findEntry(group, key, flags), option);
        }

        void setEntryOption(Iterator it, EntryOption option, bool bf);
        void setEntryOption(const QByteArray& group, const QByteArray& key, SearchFlags flags,
                            EntryOption option, bool bf)
        {
            setEntryOption(findEntry(group, key, flags), option, bf);
        }

        void revertEntry(const QByteArray& group, const QByteArray& key, SearchFlags flags=SearchFlags());
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KEntryMap::SearchFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(KEntryMap::EntryOptions)

/**
 * \relates KEntry
 * type for iterating over keys in a KEntryMap in sorted order.
 * @internal
 */
typedef QMap<KEntryKey, KEntry>::Iterator KEntryMapIterator;

/**
 * \relates KEntry
 * type for iterating over keys in a KEntryMap in sorted order.
 * It is const, thus you cannot change the entries in the iterator,
 * only examine them.
 * @internal
 */
typedef QMap<KEntryKey, KEntry>::ConstIterator KEntryMapConstIterator;

#endif
