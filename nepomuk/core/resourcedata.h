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

#ifndef _NEPOMUK_RESOURCE_DATA_H_
#define _NEPOMUK_RESOURCE_DATA_H_

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QAtomicInt>

#include "variant.h"
#include "thing.h"

#include <kurl.h>

#include <soprano/statement.h>


namespace Nepomuk {

    class ResourceManagerPrivate;

    class ResourceData : public QObject
    {
        Q_OBJECT

    public:
        explicit ResourceData( const QUrl& uri, const QString& kickoffId_, const QUrl& type_, ResourceManagerPrivate* rm );
        ~ResourceData();

        /**
         * Used instead of the destructor in Resource. The reason for the existence of
         * this method is that the destructor does not remove the uri from the global
         * data map. That behaviour is necessary since in certain situations temporary
         * ResourceData instances are created.
         */
        void deleteData();

        inline bool ref() {
            return m_ref.ref();
        }

        inline bool deref() {
            return m_ref.deref();
        }

        inline int cnt() const {
            return m_ref;
        }

        /**
         * Tries to determine if this resource represents a file by examining the type and the uri.
         */
        bool isFile();

        /**
         * The URI of the resource. This might be empty if the resource was not synced yet.
         * \sa kickoffUriOrId
         */
        QUrl uri() const;

        /**
         * \return The main type of the resource. ResourceData tries hard to make this the
         * most important type, i.e. that which is furthest down the hierachy.
         */
        QUrl type();

        QList<QUrl> allTypes();

        void setTypes( const QList<QUrl>& types );

        QHash<QUrl, Variant> allProperties();

        bool hasProperty( const QUrl& uri );

        /**
         * Does also check for subClass relations.
         */
        bool hasType( const QUrl& uri );

        Variant property( const QUrl& uri );

        /**
         * Set a property. The property will directly be saved to the RDF store.
         * Calls store to make sure this resource and property resources are properly
         * stored.
         */
        void setProperty( const QUrl& uri, const Variant& value );

        void removeProperty( const QUrl& uri );

        /**
         * Makes sure the resource is present in the RDF store. This means that if it does
         * not exist the type and the identifier (if one has been used to create the instance)
         * are stored.
         *
         * \sa exists, setProperty
         */
        bool store();

        bool load();

        /**
         * Remove this resource data from the store completely.
         * \param recursive If true all statements that contain this
         * resource as an object will be removed, too.
         */
        void remove( bool recursive = true );

        /**
         * This method only works with a proper URI, i.e. it does
         * not work on non-initialized resources that only know
         * their kickoffUriOrId
         */
        bool exists();

        bool isValid() const;

        /**
         * Makes sure the resource has a proper URI. This includes creating a new one
         * in the store if it does not exist yet.
         */
        bool determineUri();

        void invalidateCache() { m_cacheDirty = true; }

        Thing pimoThing();

        /**
         * Compares the properties of two ResourceData objects taking into account the Deleted flag
         */
        bool operator==( const ResourceData& other ) const;

        ResourceManagerPrivate* rm() const { return m_rm; }

    private:
        bool constHasType( const QUrl& type ) const;
        void loadType( const QUrl& type );

        /// identifier that was used to construct the resource. Will be used by determineUri
        /// to check for nao:identifiers or even nie:urls.
        QString m_kickoffId;

        /// the URI that was used to construct the resource. Will be used by determineUri
        /// to find the actual resource URI which is either m_kickoffUri itself or
        /// a resource URI which relates to m_kickoffUri by nie:url
        KUrl m_kickoffUri;

        /// final resource URI created by determineUri
        KUrl m_uri;

        QUrl m_mainType;
        QList<QUrl> m_types;

        QAtomicInt m_ref;

        QMutex m_determineUriMutex;
        mutable QMutex m_modificationMutex;

        /**
         * Used to virtually merge two data objects representing the same
         * resource. This happens if the resource was once created using its
         * actual URI and once via its ID. To prevent early loading we allow
         * this scenario.
         */
        ResourceData* m_proxyData;

        QHash<QUrl, Variant> m_cache;
        bool m_cacheDirty;

        // using a pointer to avoid infinite creation loop
        Thing* m_pimoThing;

        // only used for delayed storage of the pimo thing relation
        ResourceData* m_groundingOccurence;

        ResourceManagerPrivate* m_rm;
    };
}

#endif
