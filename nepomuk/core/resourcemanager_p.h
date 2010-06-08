/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2009 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _NEPOMUK_RESOURCE_MANAGER_P_H_
#define _NEPOMUK_RESOURCE_MANAGER_P_H_

#include <QtCore/QMutex>

#include <kurl.h>

#include <Soprano/Node> // for qHash(QUrl)

#include "resourcedata.h"


namespace Soprano {
    class Model;
}


namespace Nepomuk {

    class ResourceManager;
    class MainModel;
    class ResourceFilterModel;

    typedef QHash<KUrl, Nepomuk::ResourceData*> ResourceDataHash;

    class ResourceManagerPrivate
    {
    public:
        ResourceManagerPrivate( ResourceManager* manager );

        Nepomuk::MainModel* mainModel;
        ResourceFilterModel* resourceFilterModel;
        Soprano::Model* overrideModel;

        /// used to protect the initialization
        QMutex initMutex;

        /// used to protect all data in ResourceManager
        QMutex mutex;

        /// contains all initialized ResourceData object, i.e. all those which
        /// successfully ran determineUri()
        ResourceDataHash m_initializedData;

        /// contains all non-initialized ResourceData objects created in data(QUrl)
        ResourceDataHash m_uriKickoffData;

        QAtomicInt dataCnt;

        ResourceManager* m_manager;

        /**
         * The Nepomuk lib is based on the fact that for each uri only one ResourceData object is
         * created at all times. This method searches for an existing data object to reuse or creates
         * a new one if none exists.
         *
         * \param uriOrId The URI or identifier of the resource is question.
         * \type The type of the resource.
         *
         * The Resource constructors use this method in combination with ref()
         */
        ResourceData* data( const QString& uriOrId, const QUrl& type );

        /**
         * The Nepomuk lib is based on the fact that for each uri only one ResourceData object is
         * created at all times. This method searches for an existing data object to reuse or creates
         * a new one if none exists.
         *
         * \param uri The URI of the resource is question or it's nie:url or even its identified stored in
         * a QUrl object.
         * \type The type of the resource.
         *
         * The Resource constructors use this method in combination with ref()
         */
        ResourceData* data( const QUrl& uri, const QUrl& type );

        /**
         * In contrast to data(QUrl,QUrl) this method avoids the overhead of determining the resource URI
         * via ResourceData::determineUri() and simply uses \p uri as the resource URI.
         */
        ResourceData* dataForResourceUri( const QUrl& uri, const QUrl& type );

        bool dataCacheFull() const;

        /**
         * Delete unused ResourceData objects from the cache.
         * \param num The number of needed slots. The ResourceData constructor
         * uses this to make room for itself. Use < 0 to free the complete cache.
         *
         * Does NOT lock the mutex.
         */
        void cleanupCache( int num = 1 );

        bool shouldBeDeleted( ResourceData* rd ) const;

        void addToKickOffList( ResourceData* rd, const QSet<KUrl>& uris );

        QList<ResourceData*> allResourceData();
        QList<ResourceData*> allResourceDataOfType( const QUrl& type );
        QList<ResourceData*> allResourceDataWithProperty( const QUrl& _uri, const Variant& v );

        void _k_storageServiceInitialized( bool );
        void _k_dbusServiceUnregistered( const QString& serviceName );

    private:
        ResourceData* findData( const QUrl& uri );

        QUrl m_mainContext;
    };
}

#endif
