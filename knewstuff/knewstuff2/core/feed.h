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
#ifndef KNEWSTUFF2_FEED_H
#define KNEWSTUFF2_FEED_H

#include <knewstuff2/core/ktranslatable.h>
#include <knewstuff2/core/entry.h>

#include <kurl.h>

namespace KNS
{

struct FeedPrivate;

/**
 * @short KNewStuff feed.
 *
 * A feed represents a collection of entries for download. One or more feeds
 * are offered by a provider, each of which has a different characteristics.
 * Usually, GHNS providers offer three feeds, for the most popular, highest
 * rated and latest entries. Simple providers might offer no special feeds
 * at all, only one default feed will be available in this case.
 * Entries might appear in different feeds at the same time.
 *
 * @author Josef Spillner (spillner@kde.org)
 *
 * @internal
 */
class KNEWSTUFF_EXPORT Feed
{
public:
    /**
     * Constructor.
     */
    Feed();

    /**
     * Destructor.
     */
    ~Feed();

    /**
     * Sets the name for this feed.
     */
    void setName(const KTranslatable& name);

    /**
     * Retrieve the name of the feed.
     *
     * @return feed name (potentially translated)
     */
    KTranslatable name() const;

    /**
     * Sets the feed description.
     */
    void setDescription(const KTranslatable& type);

    /**
     * Retrieve the description of the feed.
     *
     * @return feed description
     */
    KTranslatable description() const;

    /**
     * Sets the URL for this feed.
     */
    void setFeedUrl(const KUrl& feedurl);

    /**
     * Retrieve the URL of the feed.
     *
     * @return URL of this feed
     */
    KUrl feedUrl() const;

    /**
     * Adds an association to an entry.
     *
     * Ownership of the entry will remain with the caller.
     *
     * @param entry Entry to link to this feed
     */
    void addEntry(Entry *entry);

    /**
     * Removes an association to an entry
     *
     * @param entry Entry to remove from this feed
     */
    void removeEntry(Entry * entry);

    /**
     * Retrieves the list of associated entries.
     *
     * If the feed loading hasn't completed yet, this method may return
     * the empty list.
     *
     * @return List of entries associated with this feed
     */
    Entry::List entries() const;

private:
    FeedPrivate * const d;
};

}

#endif
