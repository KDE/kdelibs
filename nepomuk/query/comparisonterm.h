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

#ifndef _NEPOMUK_QUERY_COMPARISON_TERM_H_
#define _NEPOMUK_QUERY_COMPARISON_TERM_H_

#include "simpleterm.h"

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Types {
        class Property;
    }

    namespace Query {
        /**
         * \class ComparisonTerm comparisonterm.h Nepomuk/Query/ComparisonTerm
         *
         * \brief ComparisonTerm
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUKQUERY_EXPORT ComparisonTerm : public SimpleTerm
        {
        public:
            enum Comparator {
                /**
                 * A LiteralTerm sub-term is matched against string literal values.
                 * Case is ignored. The literal value can contain wildcards like *.
                 */
                Contains,

                /**
                 * A LiteralTerm sub-term is matched against a string literal value
                 * using the literal term's value as a regular expression.
                 * It is highly recommended to prefer Contains over Regexp
                 * as the latter will result in much slower SPARQL REGEX filters.
                 */
                Regexp,

                /**
                 * A sub-term is matched one-to-one. Sub-terms can be any other term type.
                 */
                Equal,

                /**
                 * A LiteralTerm sub-term is matched to greater literal values.
                 */
                Greater,

                /**
                 * A LiteralTerm sub-term is matched to smaller literal values.
                 */
                Smaller,

                /**
                 * A LiteralTerm sub-term is matched to greater or equal literal values.
                 */
                GreaterOrEqual,

                /**
                 * A LiteralTerm sub-term is matched to smaller or equal literal values.
                 */
                SmallerOrEqual
            };

            ComparisonTerm();
            ComparisonTerm( const ComparisonTerm& term );
            ComparisonTerm( const Types::Property& property, const Term& term, Comparator = Contains );
            ~ComparisonTerm();

            ComparisonTerm& operator=( const ComparisonTerm& term );

            /**
             * The Comparator used by ComparisonTerm Terms.
             *
             * \sa setComparator
             */
            Comparator comparator() const;

            /**
             * A property used for ComparisonTerm Terms.
             *
             * \sa setProperty
             */
            Types::Property property() const;

            /**
             * Set the comparator
             */
            void setComparator( Comparator );

            /**
             * Set the property for ComparisonTerm
             * Terms.
             *
             * \sa property
             */
            void setProperty( const Types::Property& );
        };
    }
}

#endif
