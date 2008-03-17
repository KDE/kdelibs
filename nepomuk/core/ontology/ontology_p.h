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

#ifndef _NEPOMUK_ONTOLOGY_PRIVATE_H_
#define _NEPOMUK_ONTOLOGY_PRIVATE_H_

#include "entity_p.h"
#include "class.h"
#include "property.h"

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QSharedData>

namespace Nepomuk {
    namespace Types {
        class Class;
        class Property;

        class OntologyPrivate : public EntityPrivate
        {
        public:
            OntologyPrivate( const QUrl& uri = QUrl() );

            QList<Class> classes;
            QList<Property> properties;

            // -1 - unknown
            // 0  - no
            // 1  - yes
            int entitiesAvailable;

            bool addProperty( const QUrl& property, const Soprano::Node& value );
            bool addAncestorProperty( const QUrl& ancestorResource, const QUrl& property );

            void initEntities();
            bool loadEntities();

            void reset( bool recursive );
        };
    }
}


#include "ontology.h"
namespace Nepomuk {

    class Class;
    class Property;

    class Ontology::Private : public QSharedData
    {
    public:
        QUrl uri;
        QHash<QUrl, const Class*> classes;
        QHash<QUrl, const Property*> properties;
    };
}

#endif
