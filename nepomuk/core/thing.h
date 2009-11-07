/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>
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

#ifndef _NEPOMUK_THING_H_
#define _NEPOMUK_THING_H_

#include "resource.h"
#include "nepomuk_export.h"

// FIXME: (Would it even make sense to check in Nepomuk::Resource if a resource is a pimo:Thing and if so,
//         use the PIMO context? Or should we handle that through the Nepomuk::PimoThing class?)

namespace Nepomuk {
    /**
     * \class Thing thing.h Nepomuk/Thing
     *
     * \brief A Nepomuk PIMO Thing resource.
     *
     * A Thing represents a unique abstract concept/idea or real-world
     * entity which can have multiple representations or occurrences.
     *
     * A typical example for a Thing is a unique person resource
     * which identifies the person itself, not any representation
     * such as an addressbook entry.
     *
     * Things are typically grounded via application resources,
     * i.e. those resources that are created or handled by applications.
     * Again the typical example is the addressbook entry.
     *
     * In general it is recommended to alway use the Thing instead of the
     * grounding resource to annotate, i.e. to add tags, or relate to other
     * resources. The advantage is its uniqueness. While there can be many
     * occurrences of one concept there is only one Thing, identifying
     * uniquely.
     *
     * \author Sebastian Trueg <trueg@kde.org>
     *
     * \since 4.2
     */
    class NEPOMUK_EXPORT Thing : public Resource
    {
    public:
        /**
         * Create a Thing object with URI \p uri. If the Thing does not
         * exist in the Nepomuk storage yet, it will be created once
         * a writing method is called (such as Resource::setProperty).
         *
         * \param uri The URI of the Thing. If empty, a new random one
         * will be created.
         * \param pimoType The type of this Thing. Needs to be a subtype
         * of pimo:Thing. If empty defaults to pimo:Thing.
         */
        Thing( const QUrl& uri = QUrl(), const QUrl& pimoType = QUrl() );

        /**
         * \overload
         *
         * \param manager The resource manager to use. This allows to mix resources from different
         * managers and, thus, different models.
         *
         * \since 4.3
         */
        Thing( const QUrl& uri, const QUrl& pimoType, ResourceManager* manager );

        /**
         * Create a new Thing with label or URI \p uriOrName
         *
         * \param uriOrName The URI or the label of the Thing. The constructor
         * tries hard to find the Thing associated. If it is not found, a new
         * Thing will be created.
         * \param pimoType The type of this Thing. Needs to be a subtype
         * of pimo:Thing. If empty defaults to pimo:Thing.
         *
         * Be aware that using the other constructor is always faster in case
         * the URI of the Thing is known.
         */
        Thing( const QString& uriOrName, const QUrl& pimoType = QUrl() );

        /**
         * \overload
         *
         * \param manager The resource manager to use. This allows to mix resources from different
         * managers and, thus, different models.
         *
         * \since 4.3
         */
        Thing( const QString& uriOrName, const QUrl& pimoType, ResourceManager* manager );

        /**
         * Copy constructor.
         */
        Thing( const Thing& other );

        /**
         * Copy constructor.
         *
         * \param other the resoruce to construct the Thing from.
         */
        Thing( const Resource& other );

        /**
         * Constructor used internally.
         */
        Thing( ResourceData* );

        /**
         * Desctructor
         */
        ~Thing();

        /**
         * Assignment operator.
         */
        Thing& operator=( const Thing& res );

        /**
         * Assignment operator.
         */
        Thing& operator=( const Resource& res );

        /**
         * Assignment operator.
         */
        Thing& operator=( const QUrl& res );

        /**
         * Get the grounding occurrences for this Thing.
         * Grounding resources are physical representations
         * of the Thing.
         *
         * An example is an mp3 file which represents an audio track
         * or a website which represents a company or a person. Or the
         * addressbook entry for an abstract person thing.
         *
         * \sa pimo:groundingResource
         */
        QList<Resource> groundingOccurrences() const;

        QList<Resource> referencingOccurrences() const;

        QList<Resource> occurrences() const;

        /**
         * Add a grounding occurrence for this Thing.
         * Grounding resources are physical representations
         * of the Thing.
         *
         * An example is an mp3 file which represents an audio track
         * or a website which represents a company or a person. Or the
         * addressbook entry for an abstract person thing.
         *
         * \sa pimo:groundingResource
         */
        void addGroundingOccurrence( const Resource &res );

        /**
         * Merges two Things that represent the same real-world
         * entity.
         *
         * \param other The Thing that should be merged into this
         * Thing.
         *
         * All properties of \p other will be merged into this
         * Thing and all references to \p other will be replaced
         * with references to this Thing.
         */
//        void merge( Thing other );
    };
}

#endif
