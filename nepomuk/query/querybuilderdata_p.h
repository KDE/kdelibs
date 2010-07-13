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

#ifndef _NEPOMUK_QUERY_QUERY_BUILDER_DATA_H_
#define _NEPOMUK_QUERY_QUERY_BUILDER_DATA_H_

#include <QtCore/QString>
#include <QtCore/QLatin1String>
#include <QtCore/QSet>

#include "query.h"

namespace Nepomuk {
    namespace Query {
        class QueryBuilderData
        {
        public:
            inline QueryBuilderData( Query::SparqlFlags flags )
                : m_varNameCnt( 0 ),
                  m_flags( flags ),
                  m_depth( 0 ) {
            }

            inline Query::SparqlFlags flags() const {
                return m_flags;
            }

            inline int depth() const {
                return m_depth;
            }

            inline void increaseDepth() {
                ++m_depth;
            }

            inline void decreaseDepth() {
                --m_depth;
            }

            /// used by different implementations of TermPrivate::toSparqlGraphPattern and Query::toSparqlQuery
            inline QString uniqueVarName() {
                return QLatin1String( "?v" ) + QString::number( ++m_varNameCnt );
            }

            /// used by ComparisonTerm to add variable names set via ComparisonTerm::setVariableName
            inline void addCustomVariable( const QString& name ) {
                m_customVariables << name;
            }

            /// used by Query::toSparqlQuery
            inline QStringList customVariables() const {
                return m_customVariables.toList();
            }

            struct OrderVariable {
                OrderVariable(int w, const QString& n, Qt::SortOrder o)
                    : weight(w),
                      name(n),
                      sortOrder(o) {
                }
                int weight;
                QString name;
                Qt::SortOrder sortOrder;
            };

            /// used by ComparisonTerm to add sorting variables (names include the '?')
            inline void addOrderVariable( const QString& name, int weight, Qt::SortOrder order ) {
                int i = 0;
                while( i < m_orderVariables.count() &&
                       m_orderVariables[i].weight > weight )
                    ++i;
                m_orderVariables.insert( i, OrderVariable( weight, name, order ) );
            }

            /// used by Query::toSparqlQuery
            inline QString buildOrderString() const {
                if( m_orderVariables.isEmpty() )
                    return QString();
                QString s = QLatin1String(" ORDER BY ");
                for( int i = 0; i < m_orderVariables.count(); ++i ) {
                    if( m_orderVariables[i].sortOrder == Qt::DescendingOrder )
                        s += QLatin1String("DESC ");
                    else
                        s += QLatin1String("ASC ");
                    s += QString::fromLatin1("( %1 )").arg(m_orderVariables[i].name);
                    s += ' ';
                }
                return s;
            }

            /// create and remember a scoring variable for full text matching
            inline QString createScoringVariable() {
                QString v = uniqueVarName();
                m_scoreVariables.insert(v, m_depth);
                return v;
            }

            inline QString buildScoringExpression() const {
                QStringList scores;
                for( QHash<QString, int>::const_iterator it = m_scoreVariables.constBegin();
                     it != m_scoreVariables.constEnd(); ++it ) {
                    const QString var = it.key();
                    int depth = it.value();
                    if( depth > 0 )
                        scores += QString::fromLatin1("(%1/%2)").arg(var).arg(depth+1);
                    else
                        scores += var;
                }
                if( !scores.isEmpty() )
                    return QLatin1String("max(") + scores.join(QLatin1String("+")) + QLatin1String(") as ?_n_f_t_m_s_");
                else
                    return QString();
            }

        private:
            /// a running counter for unique variable names
            int m_varNameCnt;

            /// copy of the flags as set in Query::toSparqlQuery
            const Query::SparqlFlags m_flags;

            /// custom variables that have been added via ComparisonTerm::setVariableName
            QSet<QString> m_customVariables;

            /// variables that are used for sorting set via ComparisonTerm::setSortWeight
            QList<OrderVariable> m_orderVariables;

            /// all full-text matching scoring variables
            QHash<QString, int> m_scoreVariables;

            /// The depth of a term in the query. This is only changed by ComparisonTerm
            int m_depth;
        };
    }
}

#endif
