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

#ifndef _NEPOMUK_QUERY_NEGATION_TERM_H_
#define _NEPOMUK_QUERY_NEGATION_TERM_H_

#include "simpleterm.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Query {
        /**
         * \class NegationTerm negationterm.h Nepomuk/Query/NegationTerm
         *
         * \brief Negate an arbitrary term.
         *
         * %NegationTerm can be used to negate any other term. A typical example
         * could be to match all resources that do not have a specific tag:
         *
         * \code
         * NegationTerm::negateTerm( ComparisonTerm( Vocabulary::NAO::hasTag(), LiteralTerm("nepomuk") ) );
         * \endcode
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT NegationTerm : public SimpleTerm
        {
        public:
            /**
             * Default constructor: creates an invalid negation term.
             */
            NegationTerm();

            /**
             * Copy constructor.
             */
            NegationTerm( const NegationTerm& term );

            /**
             * Destructor.
             */
            ~NegationTerm();

            /**
             * Assignment operator.
             */
            NegationTerm& operator=( const NegationTerm& term );

            /**
             * Negate \p term. This method will either strip the first NegationTerm
             * from \p term or add one.
             *
             * \return The negated term.
             */
            static Term negateTerm( const Term& term );
        };
    }
}

#endif
