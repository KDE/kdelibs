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

#ifndef _NEPOMUK_QUERY_AND_TERM_P_H_
#define _NEPOMUK_QUERY_AND_TERM_P_H_

#include "groupterm_p.h"

namespace Nepomuk {
    namespace Query {
        class AndTermPrivate : public GroupTermPrivate
        {
        public:
            AndTermPrivate() {
                m_type = Term::And;
            }

            TermPrivate* clone() const { return new AndTermPrivate( *this ); }

            QString toSparqlGraphPattern( const QString& resourceVarName, const TermPrivate*, const QString& additionalFilters, QueryBuilderData* qbd ) const;

            /**
             * Since filters can only work on a "real" graph pattern
             * we need to make sure that such a pattern exists. This can be done by searching one in a
             * surrounding AndTerm.
             *
             * Why is that enough?
             * Nested AndTerms are flattened before the SPARQL query is constructed in Query. Thus, an AndTerm can
             * only be embedded in an OrTerm or as a child term to either a ComparisonTerm or an OptionalTerm.
             * In both cases we need a real pattern inside the AndTerm.
             *
             * This method checks if the and term has a "real" non-filter subterm.
             *
             * This method is used by NegationTerm and ResourceTerm to check if they need to add an artificial "real"
             * pattern.
             */
            bool hasRealPattern() const;
        };
    }
}

#endif
