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

#ifndef _NEPOMUK_CLASS_PRIVATE_H_
#define _NEPOMUK_CLASS_PRIVATE_H_

#include "entity_p.h"
#include "class.h"
#include "property.h"

#include <QtCore/QHash>
#include <QtCore/QSharedData>


namespace Nepomuk {
    namespace Types {
        class ClassPrivate : public EntityPrivate
        {
        public:
            ClassPrivate( const QUrl& uri = QUrl() );

            QList<Property> domainOf;
            QList<Property> rangeOf;

            QList<Class> parents;
            QList<Class> children;

            // -1 - unknown
            // 0  - no
            // 1  - yes
            int propertiesAvailable;

            bool addProperty( const QUrl& property, const Soprano::Node& value );
            bool addAncestorProperty( const QUrl& ancestorResource, const QUrl& property );

            void initProperties();
            bool loadProperties();

            void reset( bool recursive );

            // recursively find all parent or sub classes but stop at requestingClass to avoid endless loops
            QSet<Class> findParentClasses( ClassPrivate* requestingClass );
            QSet<Class> findSubClasses( ClassPrivate* requestingClass );

        protected:
            virtual bool load();
            virtual bool loadAncestors();
        };
    }
}


namespace Nepomuk {

    class Property;
    
    class Class::Private : public QSharedData
    {
    public:
        QHash<QUrl, const Property*> domainOf;
        QHash<QUrl, const Property*> rangeOf;

        QList<const Nepomuk::Class*> parents;
        QList<const Nepomuk::Class*> children;
    };
}

#endif
