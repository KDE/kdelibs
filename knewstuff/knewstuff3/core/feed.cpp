/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "feed.h"

#include <kdebug.h>

using namespace KNS;

struct KNS::FeedPrivate {
    KTranslatable mName;
    KTranslatable mDescription;
    KUrl mFeed;
    Entry::List mEntries;
};

Feed::Feed()
        : d(new FeedPrivate)
{
}

Feed::~Feed()
{
    delete d;
}

void Feed::setName(const KTranslatable& name)
{
    d->mName = name;
}

KTranslatable Feed::name() const
{
    return d->mName;
}

void Feed::setDescription(const KTranslatable &description)
{
    d->mDescription = description;
}

KTranslatable Feed::description() const
{
    return d->mDescription;
}

void Feed::setFeedUrl(const KUrl& feedurl)
{
    d->mFeed = feedurl;
}

KUrl Feed::feedUrl() const
{
    return d->mFeed;
}

void Feed::addEntry(Entry *entry)
{
    //kDebug() << "adding entry: " << entry->name().representation() << " to feed: " << this;
    d->mEntries.append(entry);
}

void Feed::removeEntry(Entry * entry)
{
    //kDebug() << "removing entry: " << entry->name().representation() << " from feed: " << this;
    d->mEntries.removeAll(entry);
}

Entry::List Feed::entries() const
{
    return d->mEntries;
}

