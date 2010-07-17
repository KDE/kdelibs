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

#ifndef _NEPOMUK_QUERY_COMPARISON_TERM_P_H_
#define _NEPOMUK_QUERY_COMPARISON_TERM_P_H_

#include "simpleterm_p.h"
#include "comparisonterm.h"

#include "property.h"

namespace Nepomuk {
    namespace Query {

        class QueryBuilderData;

        class ComparisonTermPrivate : public SimpleTermPrivate
        {
        public:
            ComparisonTermPrivate()
                : m_aggregateFunction(ComparisonTerm::NoAggregateFunction),
                  m_sortWeight(0),
                  m_sortOrder(Qt::AscendingOrder),
                  m_inverted(false) {
                m_type = Term::Comparison;
            }

            TermPrivate* clone() const { return new ComparisonTermPrivate( *this ); }

            bool equals( const TermPrivate* other ) const;
            bool isValid() const;
            QString toSparqlGraphPattern( const QString& resourceVarName, QueryBuilderData* qbd ) const;

            /**
             * return m_variableName and register it with qbd
             * or ask the latter to create a new variable.
             */
            QString getMainVariableName( QueryBuilderData* qbd ) const;

            /**
             * return the N3 form of m_property or a new variable
             * in case m_property is invalid.
             */
            QString propertyToString( QueryBuilderData* qbd ) const;

            Types::Property m_property;
            ComparisonTerm::Comparator m_comparator;

            QString m_variableName;
            ComparisonTerm::AggregateFunction m_aggregateFunction;
            int m_sortWeight;
            Qt::SortOrder m_sortOrder;
            bool m_inverted;
        };

        QString comparatorToString( Nepomuk::Query::ComparisonTerm::Comparator c );
        Nepomuk::Query::ComparisonTerm::Comparator stringToComparator( const QStringRef& c );
    }
}

#endif
