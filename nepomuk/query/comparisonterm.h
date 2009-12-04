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
         * \brief A term matching the value of a property.
         *
         * The %ComparisonTerm is the most important term in the query API. It can be
         * used to match the values of properties. As such it consists of a property(),
         * a comparator() and a subTerm().
         *
         * The latter can be any other term including AndTerm and OrTerm. The matching
         * is done in one of two ways:
         *
         * \li A LiteralTerm as subTerm() is directly matched to the value of a literal
         * property (see also Nepomuk::Types::Property::literalRangeType()) or to the
         * labels of related resources in case of a property that has a resource range.
         * \li Any other term type will be used as a sort of subquery to match the
         * related resources. This means that the property() needs to have a resource
         * range.
         *
         * For more details see the Comparator enumeration.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT ComparisonTerm : public SimpleTerm
        {
        public:
            /**
             * %ComparisonTerm supports different ways to compare values.
             */
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

            /**
             * Default constructor: creates an invalid comparison term.
             */
            ComparisonTerm();

            /**
             * Copy constructor.
             */
            ComparisonTerm( const ComparisonTerm& term );

            /**
             * Default constructor.
             *
             * \param property The property that should be matched.
             * \param term The sub term to match to.
             * \param comparator The Comparator to use for comparison. Not all Comparators make sense
             * with all sub term types.
             */
            ComparisonTerm( const Types::Property& property, const Term& term, Comparator comparator = Contains );

            /**
             * Destructor.
             */
            ~ComparisonTerm();

            /**
             * Assignment operator.
             */
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
