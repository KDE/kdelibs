/* This file is part of the Nepomuk-KDE libraries
   Copyright (c) 2007-2009 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_ENTITY_H_
#define _NEPOMUK_ENTITY_H_

#include <QtCore/QUrl>
#include <QtCore/QString>
#include <QtCore/QSharedData>
#include <QtCore/QHash>

#include <Soprano/Node>

#include <kglobal.h>
#include <klocale.h>

#include "nepomuk_export.h"

class QIcon;


namespace Nepomuk {
    namespace Types {
        class EntityPrivate;

        /**
         * \brief Abstract base class for Class and Property;
         *
         * Base class for static ontology entities Class and Property.
         * It encapsulates the generic labels and comments that both
         * types have.
         *
         * Due to internal optimizations comparing two Entities is much
         * faster than comparing two QUrl objects.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUK_EXPORT Entity
        {
        public:
            /**
             * Default copy constructor.
             */
            Entity( const Entity& );

            /**
             * Destructor.
             */
            virtual ~Entity();

            /**
             * Copy operator.
             */
            Entity& operator=( const Entity& );

            /**
             * The name of the resource. The name equals the fragment of the
             * URI.
             */
            QString name() const;

            /**
             * The URI of the resource
             */
            QUrl uri() const;

            /**
             * Retrieve the label of the entity (rdfs:label)
             *
             * \param language The code of the language to use. Defaults to the session
             *                 language configured in KDE. As of KDE 4.3 only the currently
             *                 configured language is loaded to save memory.
             *
             * \return The label translated into \p language or the default fallback label
             * if no translation is available or the name() if no label could be found
             * at all.
             */
            QString label( const QString& language = KGlobal::locale()->language() );

            /**
             * Retrieve the label of the entity (rdfs:label)
             *
             * \param language The code of the language to use. Defaults to the session
             *                 language configured in KDE. As of KDE 4.3 only the currently
             *                 configured language is loaded to save memory.
             *
             * \return The label translated into \p language or the default fallback label
             * if no translation is available or the name() if no label could be found
             * at all.
             *
             * Const version
             *
             * \since 4.4
             */
            QString label( const QString& language = KGlobal::locale()->language() ) const;

            /**
             * Retrieve the comment of the entity (rdfs:comment)
             *
             * \param language The code of the language to use. Defaults to the session
             *                 language configured in KDE. As of KDE 4.3 only the currently
             *                 configured language is loaded to save memory.
             *
             * \return The comment translated into \p language or the default fallback comment
             * if no translation is available or an empty string if no comment could be found
             * at all.
             */
            QString comment( const QString& language = KGlobal::locale()->language() );

            /**
             * Retrieve the comment of the entity (rdfs:comment)
             *
             * \param language The code of the language to use. Defaults to the session
             *                 language configured in KDE. As of KDE 4.3 only the currently
             *                 configured language is loaded to save memory.
             *
             * \return The comment translated into \p language or the default fallback comment
             * if no translation is available or an empty string if no comment could be found
             * at all.
             *
             * Const version
             *
             * \since 4.4
             */
            QString comment( const QString& language = KGlobal::locale()->language() ) const;

            /**
             * Retrieve the icon stored for the entity (nao:hasSymbol)
             *
             * If no symbol is defined for the entity a null icon will be returned.
             *
             * \since 4.1
             */
            QIcon icon();

            /**
             * Retrieve the icon stored for the entity (nao:hasSymbol)
             *
             * If no symbol is defined for the entity a null icon will be returned.
             *
             * Const version.
             *
             * \since 4.4
             */
            QIcon icon() const;

            /**
             * Is this a valid Entity, i.e. has it a valid URI.
             * A valid Entity does not necessarily have a label and a comment, it
             * does not even have to exist in the Nepomuk store.
             *
             * \sa isAvailable
             */
            bool isValid() const;

            /**
             * Is this Entity available locally, i.e. could its properties
             * be loaded from the Nepomuk store.
             */
            bool isAvailable();

            /**
             * Is this Entity available locally, i.e. could its properties
             * be loaded from the Nepomuk store.
             *
             * Const version.
             *
             * \since 4.4
             */
            bool isAvailable() const;

            /**
             * The Types classes are optimized for performance under the
             * aasumption that ontologies never change during the execution
             * time of an application.
             *
             * Since there might be situations where this does not apply
             * the internal cache can be reset via this method.
             *
             * \param recursive If \p true all related entities will be reset
             * as well.
             *
             * \since 4.1
             */
            void reset( bool recursive = false );

            /**
             * An Entity can be used as a QUrl automagically.
             */
            operator QUrl() const { return uri(); }

            /**
             * Compares two Entity instances by simply comparing their URI.
             */
            bool operator==( const Entity& other ) const;

            /**
             * Compares two Entity instances by simply comparing their URI.
             */
            bool operator!=( const Entity& other ) const;

        protected:
            /**
             * Create an invalid Entity instance.
             */
            Entity();

            QExplicitlySharedDataPointer<EntityPrivate> d;
        };

        inline uint qHash( const Entity& c )
        {
            return qHash( c.uri() );
        }
    }
}

#ifndef DISABLE_NEPOMUK_LEGACY

namespace Nepomuk {

    class Ontology;

    /**
     * \deprecated in favor of Nepomuk::Types::Entity
     */
    class KDE_DEPRECATED NEPOMUK_EXPORT Entity
    {
    public:
        Entity( const Entity& );
        ~Entity();

        Entity& operator=( const Entity& );

        /**
         * The ontology in which the resource is defined.
         */
        const Ontology* definingOntology() const;

        /**
         * The name of the resource. The name equals the fragment of the
         * URI.
         */
        QString name() const;

        /**
         * The URI of the resource
         */
        QUrl uri() const;

        QString label( const QString& language = QString() ) const;
        QString comment( const QString& language = QString() ) const;

    protected:
        Entity();

    private:
        class Private;
        QSharedDataPointer<Private> d;

        friend class OntologyManager;
    };
}

#endif // DISABLE_NEPOMUK_LEGACY

#endif // _NEPOMUK_ENTITY_H_
