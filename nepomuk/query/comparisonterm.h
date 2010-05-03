/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009-2010 Sebastian Trueg <trueg@kde.org>

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
         * used to match the values of properties. As such its core components are
         * a property(), a comparator() (see Comparator for details) and a subTerm().
         * The latter can be any other term including AndTerm, OrTerm, or even an invalid
         * term. The matching is done as follows:
         *
         * \li A LiteralTerm as subTerm() is directly matched to the value of a literal
         * property (see also Nepomuk::Types::Property::literalRangeType()) or to the
         * labels of related resources in case of a property that has a resource range.
         * \li An invalid sub term simply matches any resource or value, effectively
         * acting as a wildcard.
         * \li Any other term type will be used as a sort of subquery to match the
         * related resources. This means that the property() needs to have a resource
         * range.
         *
         * In addition to these basic features %ComparisonTerm has a few tricks up its
         * sleeve:
         *
         * \li By forcing the variable name via setVariableName() it is possible to
         * include a value matched by the term in the resulting SPARQL query (Query::toSparqlQuery())
         * or accessing it via Result::additionalBinding().
         * \li It is possible to set an aggregate function via setAggregateFunction()
         * to count or sum up the results instead of returning the actual values.
         * \li Using setSortWeight() the sorting of the results in the final query
         * can be influenced in a powerful way - especially when combined with setAggregateFunction().
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
             * Aggregate functions which can be applied to a comparison term to influence
             * the value they return.
             *
             * \sa setAggregateFunction()
             *
             * \since 4.5
             */
            enum AggregateFunction {
                /**
                 * Do not use any aggregate function.
                 */
                NoAggregateFunction = 0,

                /**
                 * Count the number of matching results instead
                 * of returning the results themselves.
                 * For counting the results of a complete query use
                 * Query::CreateCountQuery.
                 */
                Count,

                /**
                 * The same as Count except that no two similar results
                 * are counted twice.
                 */
                DistinctCount,

                /**
                 * Return the maximum value of all results instead
                 * of the results themselves.
                 * Does only make sense for numerical values.
                 */
                Max,

                /**
                 * Return the minimum value of all results instead
                 * of the results themselves.
                 * Does only make sense for numerical values.
                 */
                Min,

                /**
                 * Return the sum of all result values instead
                 * of the results themselves.
                 * Does only make sense for numerical values.
                 */
                Sum,

                /**
                 * The same as Sum except that no two similar results
                 * are added twice.
                 */
                DistinctSum,

                /**
                 * Return the average value of all results instead
                 * of the results themselves.
                 * Does only make sense for numerical values.
                 */
                Average,

                /**
                 * The same as Average except that no two similar results
                 * are counted twice.
                 */
                DistinctAverage
            };

            /**
             * Default constructor: creates a comparison term that matches all properties.
             */
            ComparisonTerm();

            /**
             * Copy constructor.
             */
            ComparisonTerm( const ComparisonTerm& term );

            /**
             * Convinience constructor which covers most simple use cases.
             *
             * \param property The property that should be matched. An invalid property will act as a wildcard.
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
             * An invalid property will act as a wildcard.
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
             * Terms. An invalid property will act as a wildcard.
             *
             * \sa property
             */
            void setProperty( const Types::Property& );

            /**
             * Set the variable name that is to be used for the
             * variable to match to. The variable will then be added
             * to the set of variables returned in the results and can
             * be read via Result::additionalBinding(). Setting
             * the variable name can be seen as a less restricted variant
             * of Query::addRequestProperty().
             *
             * When manually setting the variable name on more
             * than one ComparisonTerm there is no guarantee for the
             * uniqueness of variable names anymore which can result
             * in unwanted query results. However, this can also be
             * used deliberately in case one needs to compare the
             * same variable twice:
             *
             * \code
             * ComparisonTerm ct1( prop1, Term() );
             * ComparisonTerm ct1( prop2, Term() );
             * ct1.setVariableName( "myvar" );
             * ct2.setVariableName( "myvar" );
             * \endcode
             *
             * The above example would result in a SPARQL query
             * pattern along the lines of
             *
             * \code
             * ?r <prop1> ?myVar .
             * ?r <prop2> ?myVar .
             * \endcode
             *
             * Be aware that the variable name does not apply
             * to sub terms of types ResourceTerm or LiteralTerm.
             * In those cases the value will be ignored. The only exception
             * are LiteralTerm sub terms that are compared other than
             * with equals.
             *
             * \param name The name of the variable to be used when requesting
             * the binding via Result::additionalBinding()
             *
             * \sa Result::additionalBinding(), Query::HandleInverseProperties, \ref examples_query
             *
             * \since 4.5
             */
            void setVariableName( const QString& name );

            /**
             * The variable name set in setVariableName() or an empty
             * string if none has been set.
             *
             * \sa setVariableName()
             *
             * \since 4.5
             */
            QString variableName() const;

            /**
             * Set an aggregate function which changes the
             * result. The typical usage is to count the results instead of actually
             * returning them. For counting the results of a complete query use
             * Query::CreateCountQuery.
             *
             * \sa aggregateFunction()
             *
             * \since 4.5
             */
            void setAggregateFunction( AggregateFunction function );

            /**
             * The aggregate function to be used with the additional binding set in
             * setVariableName().
             *
             * \sa setAggregateFunction()
             *
             * \since 4.5
             */
            AggregateFunction aggregateFunction() const;

            /**
             * Set the sort weight of this property. By default all ComparisonTerms have
             * a weight of 0 which means that they are ignored for sorting. By setting \p
             * weight to a value different from 0 (typically higher than 0) the comparison
             * subterm will be used for sorting.
             *
             * Be aware that as with the variableName() sorting does not apply to sub
             * terms of types ResourceTerm or LiteralTerm.
             * In those cases the value will be ignored. The only exception
             * are LiteralTerm sub terms that are compared other than
             * with equals.
             *
             * \param weight The new sort weight. If different from 0 this term will be
             * used for sorting in the Query.
             * \param sortOrder The sort order to use for this term.
             *
             * \sa sortWeight()
             *
             * \since 4.5
             */
            void setSortWeight( int weight, Qt::SortOrder sortOrder = Qt::AscendingOrder );

            /**
             * \return The sort weight as set in setSortWeight() or 0 if
             * sorting is disabled for this term.
             *
             * \since 4.5
             */
            int sortWeight() const;

            /**
             * \return The sort order as set in setSortWeight().
             *
             * \since 4.5
             */
            Qt::SortOrder sortOrder() const;

            /**
             * \return \p true if the comparison is inverted.
             * \sa setInverted
             *
             * \since 4.5
             */
            bool isInverted() const;

            /**
             * Invert the comparison, i.e. make the subterm the subject
             * of the term and match to objects of the term.
             *
             * A typical example would be:
             *
             * \code
             * ComparisonTerm term( Soprano::Vocabulary::NAO::hasTag(),
             *                      ResourceTerm( somefile ) );
             * term.setInverted(true);
             * \endcode
             *
             * which would yield a query like the following:
             *
             * \code
             * select ?r where { <somefile> nao:hasTag ?r . }
             * \endcode
             *
             * to get all tags attached to a file.
             *
             * Be aware that this does only make sense with
             * sub terms that match to resources. When using
             * LiteralTerm as a sub term \p invert is ignored.
             *
             * \sa inverted()
             *
             * \since 4.5
             */
            void setInverted( bool invert );

            /**
             * Create an inverted copy of this %ComparisonTerm.
             * This is a convenience method to allow inline creation of
             * inverted comparison terms when creating queries in a
             * single line of code.
             *
             * Be aware that calling this method twice wil result in
             * a non-inverted comparison term:
             *
             * \code
             * // always true:
             * (term.inverted().inverted() == term);
             * \endcode
             *
             * \sa setInverted()
             *
             * \since 4.5
             */
            ComparisonTerm inverted() const;
        };
    }
}

#endif
