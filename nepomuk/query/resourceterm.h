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

#ifndef _NEPOMUK_QUERY_RESOURCE_TERM_H_
#define _NEPOMUK_QUERY_RESOURCE_TERM_H_

#include "term.h"
#include "resource.h"

#include <QtCore/QUrl>

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class ResourceTerm resourceterm.h Nepomuk/Query/ResourceTerm
         *
         * \brief Matches exactly one resource.
         *
         * The ResourceTerm does only make sense as a sub term to ComparisonTerm
         * to match to a known resource. The typical example would be matching
         * against a known tag:
         *
         * \code
         * Tag myTag = fancyGetTag();
         * ComparisonTerm term( Vocabulary::NAO::hasTag(), ResourceTerm(myTag) );
         * \endcode
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT ResourceTerm : public Term
        {
        public:
            /**
             * Copy constructor.
             */
            ResourceTerm( const ResourceTerm& term );

            /**
             * Default constructor
             *
             * \param resource The resource this term should match against.
             */
            ResourceTerm( const Nepomuk::Resource& resource = Nepomuk::Resource() );

            /**
             * Desctructor.
             */
            ~ResourceTerm();

            /**
             * Assignment operator.
             */
            ResourceTerm& operator=( const ResourceTerm& term );

            /**
             * The resource this term should match against.
             *
             * \sa setResource()
             */
            Nepomuk::Resource resource() const;

            /**
             * Set the resource this term should match against.
             *
             * \sa resource()
             */
            void setResource( const Nepomuk::Resource& resource );
        };
    }
}

#endif
