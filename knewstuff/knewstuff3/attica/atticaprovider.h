/*
    Copyright (c) 2009 Frederik Gladhorn <gladhorn@kde.org>

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
#ifndef KNEWSTUFF3_ATTICA_PROVIDER_H
#define KNEWSTUFF3_ATTICA_PROVIDER_H

#include "core/provider.h"
#include <attica/provider.h>

namespace Attica {
class BaseJob;}

namespace KNS3
{
    class AtticaProviderPrivate;
    
    /**
     * @short KNewStuff Attica Provider class.
     *
     * This class provides accessors for the provider object.
     * It should not be used directly by the application.
     * This class is the base class and will be instantiated for
     * websites that implement the Open Collaboration Services.
     *
     * @author Frederik Gladhorn <gladhorn@kde.org>
     *
     * @internal
     */
    class KNEWSTUFF_EXPORT AtticaProvider: public Provider
    {
        Q_OBJECT
    public:
        AtticaProvider(const QStringList& categories);

        ~AtticaProvider();

        virtual QString id() const;
    
        /**
         * set the provider data xml, to initialize the provider
         */
        virtual bool setProviderXML(QDomElement & xmldata);

        /**
         * get the xml for the provider
         */
        virtual QDomElement providerXML() const;

        virtual bool isInitialized() const;
        virtual void setCachedEntries(const KNS3::Entry::List& cachedEntries);
        virtual void loadEntries(SortMode sortMode = Rating, const QString & searchstring = QString(), int page = 0, int pageSize = 20);
        virtual void loadPayloadLink(const Entry& entry);
        
    private Q_SLOTS:
        void providerLoaded();
        void listOfCategoriesLoaded(Attica::BaseJob*);
        void categoryContentsLoaded(Attica::BaseJob* job);
        void downloadItemLoaded(Attica::BaseJob* job);
        void authenticationCredentialsMissing(const Provider&);
        
    protected:
        AtticaProvider(AtticaProviderPrivate &dd);

    private:
        Entry::List installedEntries() const;
        Attica::Provider::SortMode atticaSortMode(const SortMode& sortMode);
        Q_DECLARE_PRIVATE(AtticaProvider)
    };

}

#endif
