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
#ifndef KNEWSTUFF3_STATICXML_PROVIDER_H
#define KNEWSTUFF3_STATICXML_PROVIDER_H

#include "knewstuff3/core/provider.h"

namespace KNS3
{
    class StaticXmlProviderPrivate;
    
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
    class KNEWSTUFF_EXPORT StaticXmlProvider: public Provider
    {
        Q_OBJECT
    public:
        typedef QList<Provider*> List;
        /**
         * Constructor.
         */
        StaticXmlProvider();

        /**
         * Destructor.
         */
        virtual ~StaticXmlProvider();

        /**
         * set the provider data xml, to initialize the provider
         */
        virtual bool setProviderXML(QDomElement & xmldata);

        /**
         * get the xml for the provider
         */
        virtual QDomElement providerXML() const;

        virtual bool hasFeeds() const { return true; }
        virtual QStringList availableFeeds() const;
        
        /**
         * load the given feed and given page
         * @param feedname String name of the feed to load, as returned from availableFeeds()
         * @param page     page number to load
         *
         * Note: the engine connects to feedLoaded() signal to get the result
         */
        virtual void loadFeed(const QString & feedname, int page = 0);

    protected:
        StaticXmlProviderPrivate * const d_ptr;
        StaticXmlProvider(StaticXmlProviderPrivate &dd);
    private:
        Q_DECLARE_PRIVATE(StaticXmlProvider)
    };

}

#endif
