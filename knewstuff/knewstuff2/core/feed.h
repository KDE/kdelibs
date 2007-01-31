/*
    This file is part of KNewStuff2.
    Copyright (c) 2007 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF2_FEED_H
#define KNEWSTUFF2_FEED_H

#include <knewstuff2/ktranslatable.h>

#include <kurl.h>

namespace KNS {

/**
 * @short KNewStuff feed.
 *
 * @author Josef Spillner (spillner@kde.org)
 */
class Feed
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

  private:
    KTranslatable mName;
    KTranslatable mDescription;
    KUrl mFeed;

    class FeedPrivate *d;
};

}

#endif
