/* This file is part of the Nepomuk-KDE libraries
   Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_ENTITY_MANAGER_H_
#define _NEPOMUK_ENTITY_MANAGER_H_

#include <QtCore/QHash>
#include <QtCore/QUrl>
#include <QtCore/QSharedData>
#include <QtCore/QMutex>


namespace Soprano {
    class Statement;
}

namespace Nepomuk {
    namespace Types {
        class EntityPrivate;
        class ClassPrivate;
        class PropertyPrivate;
        class OntologyPrivate;

        /**
         * Cache for all loaded entities.
         */
        class EntityManager
        {
        public:
            EntityManager();

            ClassPrivate* getClass( const QUrl& uri );
            PropertyPrivate* getProperty( const QUrl& uri );
            OntologyPrivate* getOntology( const QUrl& uri );

            static EntityManager* self();

        private:
            EntityPrivate* findEntity( const QUrl& uri ) const;
            ClassPrivate* findClass( const QUrl& uri ) const;
            PropertyPrivate* findProperty( const QUrl& uri ) const;
            OntologyPrivate* findOntology( const QUrl& uri ) const;

            QHash<QUrl, QExplicitlySharedDataPointer<ClassPrivate> > m_classMap;
            QHash<QUrl, QExplicitlySharedDataPointer<PropertyPrivate> > m_propertyMap;
            QHash<QUrl, QExplicitlySharedDataPointer<OntologyPrivate> > m_ontologyMap;

            QMutex m_mutex;
        };
    }
}

#endif
