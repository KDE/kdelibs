/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_QUERY_OPTIONAL_TERM_H_
#define _NEPOMUK_QUERY_OPTIONAL_TERM_H_

#include "simpleterm.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class OptionalTerm optionalterm.h Nepomuk/Query/OptionalTerm
         *
         * \brief Make a term optional.
         *
         * %OptionalTerm can be used to mark another term as optional.
         * This is mostly useful with subterms of type ComparisonTerm
         * (or an AndTerm combining ComparisonTerms) which either have
         * additional bindings (ComparisonTerm::setVariableName()) or
         * a specified sort weight (ComparisonTerm::setSortWeight()).
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.5
         */
        class NEPOMUKQUERY_EXPORT OptionalTerm : public SimpleTerm
        {
        public:
            /**
             * Default constructor: creates an invalid optional term.
             */
            OptionalTerm();

            /**
             * Copy constructor.
             */
            OptionalTerm( const OptionalTerm& term );

            /**
             * Destructor.
             */
            ~OptionalTerm();

            /**
             * Assignment operator.
             */
            OptionalTerm& operator=( const OptionalTerm& term );

            /**
             * Mark \p term as optional. This method will encasulate
             * \p term in an OptionalTerm except if \p term is already
             * of type OptionalTerm.
             *
             * \return The optional term.
             */
            static Term optionalizeTerm( const Term& term );
        };
    }
}

#endif
