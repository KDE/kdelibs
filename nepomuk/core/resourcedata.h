/* 
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
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

#include "variant.h"
#include "thing.h"

#include <soprano/statement.h>


namespace Nepomuk {
    class ResourceData : public QObject
    {
        Q_OBJECT

    public:
        explicit ResourceData( const QUrl& uri = QUrl(), const QString& kickoffId_ = QString(), const QUrl& type_ = QString() );
        ~ResourceData();

        /**
         * Used instead of the destructor in Resource. The reason for the existence of
         * this method is that the destructor does not remove the uri from the global
         * data map. That behaviour is necessary since in certain situations temporary
         * ResourceData instances are created.
         */
        void deleteData();

        inline int ref() {
            return ++m_ref;
        }

        inline int deref() {
            return --m_ref;
        }

        inline int cnt() const {
            return m_ref;
        }

        /**
         * Until the resource has not been synced or even loaded the actual URI is not known.
         * It might even be possible that none exists yet. Thus, the identifier used to create
         * the resource object is stored in the kickoffUriOrId. During the syncing it will be
         * determined if it is an actual existing URI or an existing identifier or if a new URI
         * has to be created.
         */
        QString kickoffUriOrId() const;

        /**
         * The URI of the resource. This might be empty if the resource was not synced yet.
         * \sa kickoffUriOrId
         */
        QUrl uri() const;

        /**
         * \return The main type of the resource. ResourceData tries hard to make this the
         * most important type, i.e. that which is furthest down the hierachy.
         */
        // FIXME: return Nepomuk::Class here
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

        /**
         * Sync the local type with the type stored in the RDF store.
         * If both are compatible, i.e. both lie on one branch in the type
         * hierarchy, then the more detailed one is used. Otherwise the type is
         * not changed.
         */
//        void updateType();

        void invalidateCache() { m_cacheDirty = true; }

        Thing pimoThing();

        /**
         * Compares the properties of two ResourceData objects taking into account the Deleted flag
         */
        bool operator==( const ResourceData& other ) const;

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
        static ResourceData* data( const QString& uriOrId, const QUrl& type );

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
        static ResourceData* data( const QUrl& uri, const QUrl& type );

        static bool dataCacheFull();

        static QList<ResourceData*> allResourceData();
        static QList<ResourceData*> allResourceDataOfType( const QUrl& type );
        static QList<ResourceData*> allResourceDataWithProperty( const QUrl& _uri, const Variant& v );

    private:
        bool constHasType( const QUrl& type ) const;
        void loadType( const QUrl& type );

        /**
         * The kickoff URI or ID is used as long as the resource has not been synced yet
         * to identify it.
         */
        QString m_kickoffUriOrId;
        QUrl m_uri;


        /**
         * The kickoffIdentifier is the identifier used to construct the resource object.
         * If the object has been constructed via a URI or determineUri has not been called
         * yet this value is empty. Otherwise it equals m_kickoffUriOrId
         */
        QString m_kickoffIdentifier;
        QUrl m_mainType;
        QList<QUrl> m_types;

        int m_ref;

        QMutex m_modificationMutex;

        /**
         * Used to virtually merge two data objects representing the same
         * resource. This happens if the resource was once created using its
         * actual URI and once via its ID. To prevent early loading we allow
         * this scenario.
         */
        ResourceData* m_proxyData;

        QHash<QUrl, Variant> m_cache;
        bool m_cacheDirty;

        // used to prevent countless model operations in store()
        bool m_initialTypeSaved;

        // using a pointer to avoid infinite creation loop
        Thing* m_pimoThing;

        // only used for delayed storage of the pimo thing relation
        ResourceData* m_groundingOccurence;

        friend class ResourceManager;
    };
}

#endif
