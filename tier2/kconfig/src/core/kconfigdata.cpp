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

#include <kconfigdata.h>

QDebug operator<<(QDebug dbg, const KEntryKey& key)
{
   dbg.nospace() << "[" << key.mGroup << ", " << key.mKey << (key.bLocal?" localized":"") <<
       (key.bDefault?" default":"") << (key.bRaw?" raw":"") << "]";
   return dbg.space();
}

QDebug operator<<(QDebug dbg, const KEntry& entry)
{
   dbg.nospace() << "[" << entry.mValue << (entry.bDirty?" dirty":"") <<
       (entry.bGlobal?" global":"") << (entry.bImmutable?" immutable":"") <<
       (entry.bDeleted?" deleted":"") << (entry.bExpand?" expand":"") << "]";

   return dbg.space();
}

QMap< KEntryKey, KEntry >::Iterator KEntryMap::findExactEntry(const QByteArray& group, const QByteArray& key, KEntryMap::SearchFlags flags)
{
    KEntryKey theKey(group, key, bool(flags&SearchLocalized), bool(flags&SearchDefaults));
    return find(theKey);
}

QMap< KEntryKey, KEntry >::Iterator KEntryMap::findEntry(const QByteArray& group, const QByteArray& key, KEntryMap::SearchFlags flags)
{
    KEntryKey theKey(group, key, false, bool(flags&SearchDefaults));

    // try the localized key first
    if (flags&SearchLocalized) {
        theKey.bLocal = true;

        Iterator it = find(theKey);
        if (it != end())
            return it;

        theKey.bLocal = false;
    }
    return find(theKey);
}

QMap< KEntryKey, KEntry >::ConstIterator KEntryMap::findEntry(const QByteArray& group, const QByteArray& key, KEntryMap::SearchFlags flags) const
{
    KEntryKey theKey(group, key, false, bool(flags&SearchDefaults));

    // try the localized key first
    if (flags&SearchLocalized) {
        theKey.bLocal = true;

        ConstIterator it = find(theKey);
        if (it != constEnd())
            return it;

        theKey.bLocal = false;
    }
    return find(theKey);
}

bool KEntryMap::setEntry(const QByteArray& group, const QByteArray& key, const QByteArray& value, KEntryMap::EntryOptions options)
{
    KEntryKey k;
    KEntry e;
    bool newKey = false;

    const Iterator it = findExactEntry(group, key, SearchFlags(options>>16));

    if (key.isEmpty()) { // inserting a group marker
        k.mGroup = group;
        e.bImmutable = (options&EntryImmutable);
        if (options&EntryDeleted) {
            qWarning("Internal KConfig error: cannot mark groups as deleted");
        }
        if(it == end()) {
            insert(k, e);
            return true;
        } else if(it.value() == e) {
            return false;
        }

        it.value() = e;
        return true;
    }


    if (it != end()) {
        if (it->bImmutable)
            return false; // we cannot change this entry. Inherits group immutability.
        k = it.key();
        e = *it;
        //qDebug() << "found existing entry for key" << k;
    } else {
        // make sure the group marker is in the map
        KEntryMap const *that = this;
        ConstIterator cit = that->findEntry(group);
        if (cit == constEnd())
            insert(KEntryKey(group), KEntry());
        else if (cit->bImmutable)
            return false; // this group is immutable, so we cannot change this entry.

        k = KEntryKey(group, key);
        newKey = true;
    }

    // set these here, since we may be changing the type of key from the one we found
    k.bLocal = (options&EntryLocalized);
    k.bDefault = (options&EntryDefault);
    k.bRaw = (options&EntryRawKey);

    e.mValue = value;
    e.bDirty = e.bDirty || (options&EntryDirty);
    e.bGlobal = (options&EntryGlobal);  //we can't use || here, because changes to entries in
    //kdeglobals would be written to kdeglobals instead
    //of the local config file, regardless of the globals flag
    e.bImmutable = e.bImmutable || (options&EntryImmutable);
    if (value.isNull())
        e.bDeleted = e.bDeleted || (options&EntryDeleted);
    else
        e.bDeleted = false; // setting a value to a previously deleted entry
    e.bExpand = (options&EntryExpansion);

    if(newKey)
    {
        //qDebug() << "inserting" << k << "=" << value;
        insert(k, e);
        if(k.bDefault)
        {
            k.bDefault = false;
            //qDebug() << "also inserting" << k << "=" << value;
            insert(k, e);
        }
        // TODO check for presence of unlocalized key
        return true;
    } else {
//                KEntry e2 = it.value();
        if(it.value() != e)
        {
            //qDebug() << "changing" << k << "from" << e.mValue << "to" << value;
            it.value() = e;
            if(k.bDefault)
            {
                k.bDefault = false;
                insert(k, e);
            }
            if (!(options & EntryLocalized)) {
                KEntryKey theKey(group, key, true, false);
                //qDebug() << "non-localized entry, remove localized one:" << theKey;
                remove(theKey);
                if (k.bDefault) {
                    theKey.bDefault = false;
                    remove(theKey);
                }
            }
            return true;
        } else {
            //qDebug() << k << "was already set to" << e.mValue;
            if (!(options & EntryLocalized)) {
                //qDebug() << "unchanged non-localized entry, remove localized one.";
                KEntryKey theKey(group, key, true, false);
                bool ret = false;
                Iterator cit = find(theKey);
                if (cit != end()) {
                    erase(cit);
                    ret = true;
                }
                if (k.bDefault) {
                    theKey.bDefault = false;
                    Iterator cit = find(theKey);
                    if (cit != end()) {
                        erase(cit);
                        return true;
                    }
                }
                return ret;
            }
            //qDebug() << "localized entry, unchanged, return false";
            // When we are writing a default, we know that the non-
            // default is the same as the default, so we can simply
            // use the same branch.
            return false;
        }
    }
}

QString KEntryMap::getEntry(const QByteArray& group, const QByteArray& key, const QString& defaultValue, KEntryMap::SearchFlags flags, bool* expand) const
{
    const ConstIterator it = findEntry(group, key, flags);
    QString theValue = defaultValue;

    if (it != constEnd() && !it->bDeleted) {
        if (!it->mValue.isNull()) {
            const QByteArray data=it->mValue;
            theValue = QString::fromUtf8(data.constData(), data.length());
            if (expand)
                *expand = it->bExpand;
        }
    }

    return theValue;
}

bool KEntryMap::hasEntry(const QByteArray& group, const QByteArray& key, KEntryMap::SearchFlags flags) const
{
    const ConstIterator it = findEntry(group, key, flags);
    if (it == constEnd())
        return false;
    if (it->bDeleted)
        return false;
    if (key.isNull()) { // looking for group marker
        return it->mValue.isNull();
    }
    return true;
}

bool KEntryMap::getEntryOption(const QMap< KEntryKey, KEntry >::ConstIterator& it, KEntryMap::EntryOption option) const
{
    if (it != constEnd()) {
        switch (option) {
        case EntryDirty:
            return it->bDirty;
        case EntryLocalized:
            return it.key().bLocal;
        case EntryGlobal:
            return it->bGlobal;
        case EntryImmutable:
            return it->bImmutable;
        case EntryDeleted:
            return it->bDeleted;
        case EntryExpansion:
            return it->bExpand;
        default:
            break; // fall through
        }
    }

    return false;
}

void KEntryMap::setEntryOption(QMap< KEntryKey, KEntry >::Iterator it, KEntryMap::EntryOption option, bool bf)
{
    if (it != end()) {
        switch (option) {
        case EntryDirty:
            it->bDirty = bf;
            break;
        case EntryGlobal:
            it->bGlobal = bf;
            break;
        case EntryImmutable:
            it->bImmutable = bf;
            break;
        case EntryDeleted:
            it->bDeleted = bf;
            break;
        case EntryExpansion:
            it->bExpand = bf;
            break;
        default:
            break; // fall through
        }
    }
}

void KEntryMap::revertEntry(const QByteArray& group, const QByteArray& key, KEntryMap::SearchFlags flags)
{
    Iterator entry = findEntry(group, key, flags);
    if (entry != end()) {
        //qDebug() << "reverting [" << group << "," << key << "] = " << entry->mValue;
        const ConstIterator defaultEntry(entry+1);
        if (defaultEntry != constEnd() && defaultEntry.key().bDefault) {
            *entry = *defaultEntry;
            entry->bDirty = true;
        } else if (!entry->mValue.isNull()) {
            entry->mValue = QByteArray();
            entry->bDirty = true;
            entry->bDeleted = true;
        }
        //qDebug() << "to [" << group << "," << key << "] =" << entry->mValue;
    }
}
