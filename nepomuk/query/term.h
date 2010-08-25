/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2010 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef _NEPOMUK_SEARCH_TERM_H_
#define _NEPOMUK_SEARCH_TERM_H_

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QSharedDataPointer>

#include <Soprano/LiteralValue>

#include "nepomukquery_export.h"

namespace Nepomuk {

    class Variant;

    namespace Types {
        class Property;
    }

    namespace Query {

        class LiteralTerm;
        class ResourceTerm;
        class NegationTerm;
        class AndTerm;
        class OrTerm;
        class ComparisonTerm;
        class ResourceTypeTerm;
        class OptionalTerm;

        class TermPrivate;

        /**
         * \class Term term.h Nepomuk/Query/Term
         *
         * \brief The base class for all term types.
         *
         * Queries are build from Term instances. A Term can have one of multiple
         * types and subterms. See Term::Type for details on the different Term types.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         *
         * \since 4.4
         */
        class NEPOMUKQUERY_EXPORT Term
        {
        public:
            /**
             * The type of a term identifying its meaning.
             *
             * \sa type()
             */
            enum Type {
                /**
                 * An invalid term matching nothing
                 */
                Invalid,

                /**
                 * A literal term is the simplest form of %Term. It simply contains a literal
                 * value.
                 *
                 * \sa LiteralTerm
                 */
                Literal,

                /**
                 * A resource term matches one resource by URI.
                 *
                 * \sa ResourceTerm
                 */
                Resource,

                /**
                 * Match all resources that match all sub terms.
                 *
                 * \sa AndTerm
                 */
                And,

                /**
                 * Match all resources that match one of the sub terms.
                 *
                 * \sa OrTerm
                 */
                Or,

                /**
                 * A comparison. The comparison operator needs to be specified in addition.
                 * A single subterm specifies the resource or value to match.
                 *
                 * \sa ComparisonTerm
                 */
                Comparison,

                /**
                 * Matches all resources of a specific type.
                 *
                 * \sa ResourceTypeTerm
                 */
                ResourceType,

                /**
                 * A negation term inverts the meaning of its sub term.
                 *
                 * \sa NegationTerm
                 */
                Negation,

                /**
                 * An optional term which marks its sub term as optional.
                 *
                 * \sa OptionalTerm
                 *
                 * \since 4.5
                 */
                Optional
            };

            /**
             * Constructs an invalid term.
             */
            Term();

            /**
             * Copy constructor.
             */
            Term( const Term& other );

            /**
             * Destructor
             */
            ~Term();

            /**
             * Copy operator.
             */
            Term& operator=( const Term& other );

            /**
             * \return \p true if the term is valid. A valid
             * term can be used to construct a Query.
             */
            bool isValid() const;

            /**
             * \return the Term type.
             */
            Type type() const;

            /**
             * \return \p true if this term is a LiteralTerm.
             */
            bool isLiteralTerm() const;

            /**
             * \return \p true if this term is a ResourceTerm.
             */
            bool isResourceTerm() const;

            /**
             * \return \p true if this term is a NegationTerm.
             */
            bool isNegationTerm() const;

            /**
             * \return \p true if this term is a OptionalTerm.
             *
             * \since 4.5
             */
            bool isOptionalTerm() const;

            /**
             * \return \p true if this term is an AndTerm.
             */
            bool isAndTerm() const;

            /**
             * \return \p true if this term is an OrTerm.
             */
            bool isOrTerm() const;

            /**
             * \return \p true if this term is a ComparisonTerm.
             */
            bool isComparisonTerm() const;

            /**
             * \return \p true if this term is a ResourceTypeTerm.
             */
            bool isResourceTypeTerm() const;

            /**
             * Interpret this term as a LiteralTerm.
             *
             * \return A copy of this term if its type
             * is Literal, a new LiteralTerm otherwise.
             */
            LiteralTerm toLiteralTerm() const;

            /**
             * Interpret this term as a ResourceTerm.
             *
             * \return A copy of this term if its type
             * is Resource, a new ResourceTerm otherwise.
             */
            ResourceTerm toResourceTerm() const;

            /**
             * Interpret this term as a NegationTerm.
             *
             * \return A copy of this term if its type
             * is Negation, a new NegationTerm otherwise.
             */
            NegationTerm toNegationTerm() const;

            /**
             * Interpret this term as a OptionalTerm.
             *
             * \return A copy of this term if its type
             * is Optional, a new OptionalTerm otherwise.
             *
             * \since 4.5
             */
            OptionalTerm toOptionalTerm() const;

            /**
             * Interpret this term as a AndTerm.
             *
             * \return A copy of this term if its type
             * is And, a new AndTerm otherwise.
             */
            AndTerm toAndTerm() const;

            /**
             * Interpret this term as a OrTerm.
             *
             * \return A copy of this term if its type
             * is Or, a new OrTerm otherwise.
             */
            OrTerm toOrTerm() const;

            /**
             * Interpret this term as a ComparisonTerm.
             *
             * \return A copy of this term if its type
             * is Comparison, a new ComparisonTerm otherwise.
             */
            ComparisonTerm toComparisonTerm() const;

            /**
             * Interpret this term as a ResourceTypeTerm.
             *
             * \return A copy of this term if its type
             * is ClassType, a new ResourceTypeTerm otherwise.
             */
            ResourceTypeTerm toResourceTypeTerm() const;

            /**
             * Convert this term into a LiteralTerm.
             * If type is not Literal it will be changed
             * and the result is a new LiteralTerm.
             *
             * \return A reference to this term as a LiteralTerm.
             */
            LiteralTerm& toLiteralTerm();

            /**
             * Convert this term into a ResourceTerm.
             * If type is not Resource it will be changed
             * and the result is a new ResourceTerm.
             *
             * \return A reference to this term as a ResourceTerm.
             */
            ResourceTerm& toResourceTerm();

            /**
             * Convert this term into a NegationTerm.
             * If type is not Negation it will be changed
             * and the result is a new NegationTerm.
             *
             * \return A reference to this term as a NegationTerm.
             */
            NegationTerm& toNegationTerm();

            /**
             * Convert this term into a OptionalTerm.
             * If type is not Optional it will be changed
             * and the result is a new OptionalTerm.
             *
             * \return A reference to this term as a OptionalTerm.
             *
             * \since 4.5
             */
            OptionalTerm& toOptionalTerm();

            /**
             * Convert this term into a AndTerm.
             * If type is not And it will be changed
             * and the result is a new AndTerm.
             *
             * \return A reference to this term as a AndTerm.
             */
            AndTerm& toAndTerm();

            /**
             * Convert this term into a OrTerm.
             * If type is not Or it will be changed
             * and the result is a new OrTerm.
             *
             * \return A reference to this term as a OrTerm.
             */
            OrTerm& toOrTerm();

            /**
             * Convert this term into a ComparisonTerm.
             * If type is not Comparison it will be changed
             * and the result is a new ComparisonTerm.
             *
             * \return A reference to this term as a ComparisonTerm.
             */
            ComparisonTerm& toComparisonTerm();

            /**
             * Convert this term into a ResourceTypeTerm.
             * If type is not ClassType it will be changed
             * and the result is a new ResourceTypeTerm.
             *
             * \return A reference to this term as a ResourceTypeTerm.
             */
            ResourceTypeTerm& toResourceTypeTerm();

            /**
             * Encode the Term in a string. Be aware that this does NOT create a SPARQL
             * query. The returned string can be used to serialize terms that can later
             * be read via fromString().
             *
             * \sa fromString()
             *
             * \since 4.5
             */
            QString toString() const;

            /**
             * Parse a Term that has been encoded as a string via toString().
             *
             * \warning This method can NOT parse SPARQL syntax.
             *
             * \sa toString()
             *
             * \since 4.5
             */
            static Term fromString( const QString& s );

            /**
             * Construct a Term from a Variant value. This is a convenience method
             * that simplifies handling Nepomuk values. However, list variants are
             * not supported and will result in an invalid Term.
             *
             * \return A ResourceTerm in case \p variant is a resource, a LiteralTerm
             * if \p variant is a supported literal value, or an invalid Term if \p
             * variant is invalid or a list.
             *
             * \since 4.6
             */
            static Term fromVariant( const Variant& variant );

            /**
             * Create a term using a Types::Property and a Variant. Sadly this cannot be modelled
             * as an operator since it would clash with Entity::operator==().
             *
             * \param property The property to be used in the ComparisonTerm.
             * \param variant The value to be compared to. Either ResourceTerm or LiteralTerm is used. List
             * variants (Variant::isList()) are handled via an AndTerm meaning all values need to match.
             *
             * \since 4.6
             *
             * \relates ComparisonTerm
             */
            static Term fromProperty( const Types::Property& property, const Variant& variant );

            /**
             * Comparison operator.
             *
             * \return \p true if this term is equal to \p term.
             */
            bool operator==( const Term& term ) const;

            /**
             * Comparison operator.
             *
             * \return \p true if this term differs from \p term.
             *
             * \since 4.6
             */
            bool operator!=( const Term& term ) const;

            // FIXME: the compiler does not find this operator!
            QDebug operator<<( QDebug ) const;

            /** \cond protected_term_members */
        protected:
            Term( TermPrivate* );

            QSharedDataPointer<TermPrivate> d_ptr;

            friend class TermPrivate;
            friend class GroupTermPrivate;
            friend class AndTermPrivate;
            friend class OrTermPrivate;
            friend class ComparisonTermPrivate;
            friend class NegationTermPrivate;
            friend class OptionalTermPrivate;
            friend class Query;
            /** \endcond */
        };

        /**
         * Logical and operator which combines two terms into
         * one term matching both \p term1 and \p term2.
         *
         * \relates AndTerm
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Term operator&&( const Term& term1, const Term& term2 );

        /**
         * Logical or operator which combines two terms into
         * one term matching either \p term1 or \p term2.
         *
         * \relates OrTerm
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Term operator||( const Term& term1, const Term& term2 );

        /**
         * Logical negation operator which negates the meaning of
         * one term.
         *
         * \sa NegationTerm::negateTerm()
         * \relates NegationTerm
         *
         * \since 4.6
         */
        NEPOMUKQUERY_EXPORT Term operator!( const Term& term );

        /**
         * Comparision operator for simple creation of ComparisonTerm objects using the
         * ComparisonTerm::Smaller comparator.
         *
         * \return A ComparisonTerm equvalent to:
         *
         * \code
         * ComparisonTerm( property, term, ComparisonTerm::Smaller );
         * \endcode
         *
         * \since 4.6
         *
         * \relates ComparisonTerm
         */
        NEPOMUKQUERY_EXPORT ComparisonTerm operator<( const Types::Property& property, const Term& term );

        /**
         * Comparision operator for simple creation of ComparisonTerm objects using the
         * ComparisonTerm::Greater comparator.
         *
         * \return A ComparisonTerm equvalent to:
         *
         * \code
         * ComparisonTerm( property, term, ComparisonTerm::Greater );
         * \endcode
         *
         * \since 4.6
         *
         * \relates ComparisonTerm
         */
        NEPOMUKQUERY_EXPORT ComparisonTerm operator>( const Types::Property& property, const Term& term );

        /**
         * Comparision operator for simple creation of ComparisonTerm objects using the
         * ComparisonTerm::SmallerOrEqual comparator.
         *
         * \return A ComparisonTerm equvalent to:
         *
         * \code
         * ComparisonTerm( property, term, ComparisonTerm::SmallerOrEqual );
         * \endcode
         *
         * \since 4.6
         *
         * \relates ComparisonTerm
         */
        NEPOMUKQUERY_EXPORT ComparisonTerm operator<=( const Types::Property& property, const Term& term );

        /**
         * Comparision operator for simple creation of ComparisonTerm objects using the
         * ComparisonTerm::GreaterOrEqual comparator.
         *
         * \return A ComparisonTerm equvalent to:
         *
         * \code
         * ComparisonTerm( property, term, ComparisonTerm::GreaterOrEqual );
         * \endcode
         *
         * \since 4.6
         *
         * \relates ComparisonTerm
         */
        NEPOMUKQUERY_EXPORT ComparisonTerm operator>=( const Types::Property& property, const Term& term );

        /**
         * Comparision operator for simple creation of ComparisonTerm objects using the
         * ComparisonTerm::Equal comparator.
         *
         * \return A ComparisonTerm equvalent to:
         *
         * \code
         * ComparisonTerm( property, term, ComparisonTerm::Equal );
         * \endcode
         *
         * \since 4.6
         *
         * \relates ComparisonTerm
         */
        NEPOMUKQUERY_EXPORT ComparisonTerm operator==( const Types::Property& property, const Term& term );

        /**
         * Comparision operator for simple creation of negated ComparisionTerm objects using the
         * ComparisonTerm::Equal comparator.
         *
         * \since 4.6
         *
         * \return A Term equvalent to:
         *
         * \code
         * NegationTerm::negateTerm( ComparisonTerm( property, term, ComparisonTerm::Equal ) );
         * \endcode
         *
         * \relates ComparisonTerm
         *
         * \sa NegationTerm
         */
        NEPOMUKQUERY_EXPORT Term operator!=( const Types::Property& property, const Term& term );

        NEPOMUKQUERY_EXPORT uint qHash( const Nepomuk::Query::Term& );
    }

}

/** \cond hide_nepomuk_term_clone_from_doxygen */
// there is a hand written instantiation of clone()
template<> Nepomuk::Query::TermPrivate* QSharedDataPointer<Nepomuk::Query::TermPrivate>::clone();
/** \endcond */

// FIXME: the compiler does not find the operator in the class
NEPOMUKQUERY_EXPORT QDebug operator<<( QDebug, const Nepomuk::Query::Term& );

#endif
