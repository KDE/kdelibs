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
#include <QtCore/QStack>

#include "literal.h"
#include "query.h"
#include "query_p.h"
#include "groupterm_p.h"

namespace Nepomuk {
    namespace Query {
        class QueryBuilderData
        {
        private:
            /// the query we are working for
            const QueryPrivate* m_query;

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

            /// see uniqueVarName()
            struct GroupTermPropertyCache {
                const GroupTermPrivate* term;
                QHash<Types::Property, QString> variableNameHash;
            };

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

            /// a stack of the group terms, the top item is the group the currently handled term is in
            QStack<GroupTermPropertyCache> m_groupTermStack;

            /// full text search terms with depth 0 for which bif:search_excerpt will be used
            QHash<QString, QString> m_fullTextSearchTerms;

        public:
            inline QueryBuilderData( const QueryPrivate* query, Query::SparqlFlags flags )
                : m_query( query ),
                  m_varNameCnt( 0 ),
                  m_flags( flags ),
                  m_depth( 0 ) {
            }

            inline const QueryPrivate* query() const {
                return m_query;
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
            ///
            /// we use a simple query optimization trick here that Virtuoso cannot pull itself since it does
            /// not know about NRL cardinalities:
            /// In one group term we can use the same variable name for comparisons on the same property with a
            /// cardinality of 1. This reduces the number of match candidates for Virtuoso, thus, significantly
            /// speeding up the query.
            inline QString uniqueVarName( const Types::Property& property = Types::Property() ) {
                if( property.isValid() &&
                    property.maxCardinality() == 1 &&
                    !m_groupTermStack.isEmpty() ) {
                    // use only one variable name for all occurrences of this property
                    GroupTermPropertyCache& gpc = m_groupTermStack.top();
                    QHash<Types::Property, QString>::const_iterator it = gpc.variableNameHash.constFind( property );
                    if( it == gpc.variableNameHash.constEnd() ) {
                        QString v = QLatin1String( "?v" ) + QString::number( ++m_varNameCnt );
                        gpc.variableNameHash.insert( property, v );
                        return v;
                    }
                    else {
                        return *it;
                    }
                }
                else {
                    return QLatin1String( "?v" ) + QString::number( ++m_varNameCnt );
                }
            }

            /// used by ComparisonTerm to add variable names set via ComparisonTerm::setVariableName
            inline void addCustomVariable( const QString& name ) {
                m_customVariables << name;
            }

            /// used by Query::toSparqlQuery
            inline QStringList customVariables() const {
                return m_customVariables.toList();
            }

            /// used by ComparisonTerm to add sorting variables (names include the '?')
            inline void addOrderVariable( const QString& name, int weight, Qt::SortOrder order ) {
                int i = 0;
                while( i < m_orderVariables.count() &&
                       m_orderVariables[i].weight > weight )
                    ++i;
                m_orderVariables.insert( i, OrderVariable( weight, name, order ) );
            }

            /// used by LiteralTerm and ComparisonTerm
            /// states that "varName" is a value matching the full text search "text"
            inline void addFullTextSearchTerm( const QString& varName, const QString& text ) {
                m_fullTextSearchTerms.insert( varName, text );
            }

            /// used by AndTermPrivate and OrTermPrivate in toSparqlGraphPattern
            inline void pushGroupTerm( const GroupTermPrivate* group ) {
                GroupTermPropertyCache gpc;
                gpc.term = group;
                m_groupTermStack.push( gpc );
            }

            /// used by AndTermPrivate and OrTermPrivate in toSparqlGraphPattern
            inline void popGroupTerm() {
                m_groupTermStack.pop();
            }

            /// used by Query::toSparqlQuery
            inline QString buildOrderString() const {
                QList<OrderVariable> orderVars = m_orderVariables;
                if( m_query->m_fullTextScoringEnabled &&
                    !m_fullTextSearchTerms.isEmpty() ) {
                    orderVars.append( OrderVariable( 0, QLatin1String("?_n_f_t_m_s_"), m_query->m_fullTextScoringSortOrder ) );
                }

                if( !orderVars.isEmpty() ) {
                    QStringList s;
                    Q_FOREACH( const OrderVariable& orderVar, orderVars ) {
                        if( orderVar.sortOrder == Qt::DescendingOrder )
                            s += QLatin1String("DESC ");
                        else
                            s += QLatin1String("ASC ");
                        s += QString::fromLatin1("( %1 )").arg(orderVar.name);
                    }
                    return QLatin1String(" ORDER BY ") + s.join( QLatin1String(" ") );
                }
                else {
                    return QString();
                }
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

            inline QString buildSearchExcerptExpression() const {
                if( !m_fullTextSearchTerms.isEmpty() ) {
                    QStringList excerptParts;
                    for( QHash<QString, QString>::const_iterator it = m_fullTextSearchTerms.constBegin();
                         it != m_fullTextSearchTerms.constEnd(); ++it ) {
                        const QString& varName = it.key();
                        const QString& text = it.value();
                        // bif:search_excerpt wants a vector of all search terms, thus, we split by spaces
                        // and remove the quotes added by LiteralTermPrivate::queryText()
                        const QStringList terms = text.mid(1, text.length()-2).split(' ');
                        excerptParts
                            << QString::fromLatin1("bif:search_excerpt(bif:vector('%1'), %2)")
                            .arg( terms.join(QLatin1String("','")),
                                  varName );
                    }

                    return QString::fromLatin1("(bif:concat(%1)) as ?_n_f_t_m_ex_")
                        .arg(excerptParts.join(QLatin1String(",")));
                }
                else {
                    return QString();
                }
            }
        };
    }
}

#endif
