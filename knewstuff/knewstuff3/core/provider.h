/*
    knewstuff3/provider.h
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
#ifndef KNEWSTUFF3_PROVIDER_H
#define KNEWSTUFF3_PROVIDER_H

#include "knewstuff3/core/entry.h"
#include "knewstuff3/core/ktranslatable.h"

#include <kurl.h>

#include <QtCore/QList>
#include <QtCore/QString>

namespace KNS3
{
    class ProviderPrivate;
    
    /**
     * @short KNewStuff Base Provider class.
     *
     * This class provides accessors for the provider object.
     * It should not be used directly by the application.
     * This class is the base class and will be instantiated for
     * static website providers.
     *
     * @author Jeremy Whiting <jpwhiting@kde.org>
     *
     * @internal
     */
    class KNEWSTUFF_EXPORT Provider: public QObject
    {
        Q_OBJECT
    public:
        typedef QList<Provider*> List;

        enum SortMode {
            Newest,
            Alphabetical,
            Rating,
            Downloads
        };
    
        /**
         * Constructor.
         */
        Provider();
        Provider(const Provider &other);

        
        /**
         * Destructor.
         */
        virtual ~Provider();

        /**
         * A unique Id for this provider (the url in most cases)
         */
        virtual QString id() const = 0;
        
        /**
         * set the provider data xml, to initialize the provider
         */
        virtual bool setProviderXML(QDomElement & xmldata) = 0;

        /**
         * get the xml for the provider
         */
        virtual QDomElement providerXML() const = 0;

        virtual bool isInitialized() const = 0;
        
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
        virtual KUrl icon() const; // FIXME use KIcon or pixmap?

        /**
         * load the given search and return given page
         * @param sortMode string to select the order in which the results are presented
         * @param searchstring string to search with
         * @param page         page number to load
         *
         * Note: the engine connects to loadingFinished() signal to get the result
         */
        virtual void loadEntries(SortMode sortMode = Rating, const QString & searchstring = QString(), int page = 0, int pageSize = 100) = 0;
        virtual void loadPayloadLink(const Entry& entry) = 0;

        virtual bool hasCommenting() const { return false; }
        virtual void getComments(Entry *, int page = 0) { Q_UNUSED(page) }
        virtual void addComment(Entry*, const QString & comment) { Q_UNUSED(comment) }

        virtual bool hasRatings() const { return false; }
        virtual void setRating(Entry*, int) {}
        
    signals:
        void providerInitialized(KNS3::Provider*);
            
        void loadingFinished(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page, int pageSize, int totalpages, Entry::List) const;
        void loadingFailed(KNS3::Provider::SortMode sortMode, const QString& searchstring, int page);

        void comments(Entry *, int page, int pageSize, int totalpages);
        void commentAdded(Entry *);
        void commentAddFailed(Entry *);

        void ratingSet(Entry*);
        void ratingSetFailed(Entry *);

        void payloadLinkLoaded(const Entry& entry);
        
    protected:
        ProviderPrivate * const d_ptr;
        Provider(ProviderPrivate &dd);
    private:
        Q_DECLARE_PRIVATE(Provider)
    };

}

#endif
