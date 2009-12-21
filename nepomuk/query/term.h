/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2009 Sebastian Trueg <trueg@kde.org>

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
    namespace Query {

        class LiteralTerm;
        class ResourceTerm;
        class NegationTerm;
        class AndTerm;
        class OrTerm;
        class ComparisonTerm;
        class ResourceTypeTerm;

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
                Negation
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
             * Comparison operator.
             *
             * \return \p true if this term is equal to \p term.
             */
            bool operator==( const Term& term ) const;

            // FIXME: the compiler does not find this operator!
            QDebug operator<<( QDebug ) const;

            /** \cond protected_error_members */
        protected:
            Term( TermPrivate* );

            QSharedDataPointer<TermPrivate> d_ptr;

            friend class TermPrivate;
            friend class GroupTermPrivate;
            friend class AndTermPrivate;
            friend class OrTermPrivate;
            friend class ComparisonTermPrivate;
            friend class NegationTermPrivate;
            friend class Query;
            /** \endcond */
        };

        NEPOMUKQUERY_EXPORT uint qHash( const Nepomuk::Query::Term& );
    }
}

// there is a hand written instantiation of clone()
template<> Nepomuk::Query::TermPrivate* QSharedDataPointer<Nepomuk::Query::TermPrivate>::clone();

// FIXME: the compiler does not find the operator in the class
NEPOMUKQUERY_EXPORT QDebug operator<<( QDebug, const Nepomuk::Query::Term& );

#endif
