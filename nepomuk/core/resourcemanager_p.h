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
#include <QtCore/QUrl>

#include "resourcedata.h"


namespace Soprano {
    class Model;
}


namespace Nepomuk {

    class ResourceManager;
    class MainModel;
    class ResourceFilterModel;

    typedef QHash<QString, Nepomuk::ResourceData*> ResourceDataHash;

    class ResourceManagerPrivate
    {
    public:
        ResourceManagerPrivate( ResourceManager* manager );

        /**
         * The Nepomuk lib uses just one context (named graph)
         * for all statements. This makes things simpler and
         * we do not use the context for anything else than
         * the named graph type anyway.
         */
        QUrl mainContext();

        Nepomuk::MainModel* mainModel;
        ResourceFilterModel* resourceFilterModel;
        Soprano::Model* overrideModel;

        QMutex mutex;

        ResourceDataHash m_initializedData;
        ResourceDataHash m_kickoffData;
        int dataCnt;

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
         * \param uri The URI of the resource is question.
         * \type The type of the resource.
         *
         * The Resource constructors use this method in combination with ref()
         */
        ResourceData* data( const QUrl& uri, const QUrl& type );

        bool dataCacheFull();
        void cleanupCache();

        QList<ResourceData*> allResourceData();
        QList<ResourceData*> allResourceDataOfType( const QUrl& type );
        QList<ResourceData*> allResourceDataWithProperty( const QUrl& _uri, const Variant& v );

    private:
        QUrl m_mainContext;
    };
}

#endif
