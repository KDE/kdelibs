/*
    knewstuff2/providerbase.h
    This file is part of KNewStuff2.
    Copyright (c) 2009 Jeremy Whiting <jpwhiting@kde.org>

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
#ifndef KNEWSTUFF2_PROVIDER_H
#define KNEWSTUFF2_PROVIDER_H

#include "core/entry.h"
#include "core/ktranslatable.h"

#include <kurl.h>

//#include <QtXml/qdom.h>
//#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

//namespace KIO
//{
//class Job;
//}

namespace KNS
{

    /**
     * @short KNewStuff Base Provider class.
     *
     * This class provides accessors for the provider object.
     * It should probably not be used directly by the application.
     * This class is the base class and will be instantiated for
     * static website providers.
     *
     * @author Jeremy Whiting <jpwhiting@kde.org>
     *
     * @internal
     */
    class KNEWSTUFF_EXPORT ProviderBase
    {
    public:
        /**
         * Constructor.
         */
        ProviderBase();

        /**
         * Destructor.
         */
        ~ProviderBase();

        /**
         * set the provider data xml, to initialize the provider
         */
        virtual void setProviderData(const QString & xmldata);

        /**
         * Retrieves the common name of the provider.
         *
         * @return provider name
         */
        virtual KTranslatable name() const;

        /**
         * Retrieves the icon URL for this provider.
         *
         * @return icon URL
         */
        KUrl icon() const;

        virtual bool hasFeeds() const { return true; }
        virtual QStringList availableFeeds() const;
        
        /*
         * load the given feed and given page
         * @param feedname String name of the feed to load, as returned from availableFeeds()
         * @param page     page number to load
         *
         * Note: the engine connects to feedLoaded() signal to get the result
         */
        virtual void loadFeed(const QString & feedname, int page = 0);

        virtual bool hasSearch() const { return false; }

        /*
         * load the given search and return given page
         * @param searchstring string to search with
         * @param page         page number to load
         *
         * Note: the engine connects to searchResults() signal to get the result
         */
        virtual void loadSearch(const QString & searchstring, int page = 0) {}

        virtual bool hasCommenting() const { return false; }
        virtual void getComments(Entry *, int page = 0) {}
        virtual void addComment(Entry*, const QString & comment) {}

        virtual bool hasRatings() const { return false; }
        virtual void getRating(Entry*) {}
        virtual void setRating(Entry*, int) {}

    signals:
        void feedLoaded(QString feedname, int page, int totalpages, Entry::List);
        void searchResults(QString searchstring, int page, int totalpages, Entry::List);

        void comments(Entry *, int page, int totalpages);
        void commentAdded(Entry *);
        void commentAddFailed(Entry *);

        void entryRating(Entry*, int rating);
        void ratingSet(Entry*);
        void ratingSetFailed(Entry *);

    protected:
        class ProviderBasePrivate *d;
    };

}

#endif
