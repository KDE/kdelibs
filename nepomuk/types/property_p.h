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

#ifndef _NEPOMUK_PROPERTY_PRIVATE_H_
#define _NEPOMUK_PROPERTY_PRIVATE_H_

#include "entity_p.h"
#include "property.h"
#include "literal.h"
#include "class.h"

#include <QtCore/QVariant>
#include <QtCore/QSharedData>

namespace Nepomuk {
    namespace Types {
        class PropertyPrivate : public EntityPrivate
        {
        public:
            PropertyPrivate( const QUrl& uri = QUrl() );

            Class range;
            Class domain;

            Literal literalRange;

            Property inverse;

            QList<Property> parents;
            QList<Property> children;

            int minCardinality;
            int maxCardinality;
            int cardinality;

            bool addProperty( const QUrl& property, const Soprano::Node& value );
            bool addAncestorProperty( const QUrl& ancestorResource, const QUrl& property );

            void reset( bool recursive );
        };
    }
}



namespace Nepomuk {

    class Class;

    class Property::Private : public QSharedData
    {
    public:
        Private()
            : range( 0 ),
            domain( 0 ),
            inverse( 0 ),
            minCardinality( -1 ),
            maxCardinality( -1 ),
            cardinality( -1 ) {
	    }

	    const Class* range;
	    const Class* domain;

	    Literal literalRange;

	    const Property* inverse;

	    QList<const Property*> parents;
	    QList<const Property*> children;

	    int minCardinality;
	    int maxCardinality;
	    int cardinality;
    };
}

#endif
