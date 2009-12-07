/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NEPOMUK_QUERY_RESOURCE_TYPE_TERM_H_
#define _NEPOMUK_QUERY_RESOURCE_TYPE_TERM_H_

#include "term.h"
#include "class.h"

#include <QtCore/QUrl>

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class ResourceTypeTerm resourcetypeterm.h Nepomuk/Query/ResourceTypeTerm
         *
         * \brief Matching resources by type.
         *
         * The %ResourceTypeTerm matches all resources of a specific type. The only
         * difference to using a ComparisonTerm with property \p rdf:type and a ResourceTerm
         * subterm is that %ResourceTypeTerm also matches resources which have a type
         * derived from the requested one.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT ResourceTypeTerm : public Term
        {
        public:
            /**
             * Copy constructor.
             */
            ResourceTypeTerm( const ResourceTypeTerm& term );

            /**
             * Default constructor.
             *
             * \param type The type to match.
             */
            ResourceTypeTerm( const Nepomuk::Types::Class& type = Nepomuk::Types::Class() );

            /**
             * Destructor.
             */
            ~ResourceTypeTerm();

            /**
             * Assignment operator.
             */
            ResourceTypeTerm& operator=( const ResourceTypeTerm& term );

            /**
             * The type to match.
             *
             * \sa setType()
             */
            Nepomuk::Types::Class type() const;

            /**
             * Set the type to match.
             *
             * \sa type()
             */
            void setType( const Nepomuk::Types::Class& type );
        };
    }
}

#endif
