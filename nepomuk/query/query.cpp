/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>

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

#include "query.h"
#include "query_p.h"
#include "term.h"
#include "term_p.h"
#include "nie.h"
#include "nfo.h"
#include "querybuilderdata_p.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"
#include "comparisonterm.h"
#include "resourcetypeterm.h"

#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QRegExp>
#include <QtCore/QVariant>

#include <Soprano/Node>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>

#include "resourcemanager.h"
#include "literal.h"
#include "property.h"
#include "class.h"

#include <kdebug.h>

/*
## Full Text Score
## Entity Rank
## Surfaced via SPARQL

SELECT ?s ?sc  ( <LONG::IRI_RANK> (?s) ) as ?Rank
WHERE { ?s ?p ?o .
?o bif:contains 'NEW AND YOR' option (score ?sc). }
ORDER BY  desc (?
*/



QString Nepomuk::Query::QueryPrivate::createFolderFilter( const QString& resourceVarName, QueryBuilderData* qbd ) const
{
    if ( m_includeFolders.count() + m_excludeFolders.count() ) {
        QString uriVarName = qbd->uniqueVarName();
        QString filter = resourceVarName + ' ' + Soprano::Node::resourceToN3( Nepomuk::Vocabulary::NIE::url() ) + ' ' + uriVarName + QLatin1String( " . " );
        if ( !m_includeFolders.isEmpty() ) {
            filter += QString( " FILTER(REGEX(STR(%1), \"^(%2/)\")) ." ).arg( uriVarName ).arg( m_includeFolders.toStringList().join( "|" ) );
        }
        if ( !m_excludeFolders.isEmpty() ) {
            filter += QString( " FILTER(!REGEX(STR(%1), \"^(%2/)\")) ." ).arg( uriVarName ).arg( m_excludeFolders.toStringList().join( "|" ) );
        }
        return filter;
    }
    else {
        return QString();
    }
}



QString Nepomuk::Query::QueryPrivate::buildRequestPropertyVariableList() const
{
    QString s;
    for ( int i = 1; i <= m_requestProperties.count(); ++i ) {
        s += QString( "?reqProp%1 " ).arg( i );
    }
    return s;
}


QString Nepomuk::Query::QueryPrivate::buildRequestPropertyPatterns() const
{
    QString s;
    int i = 1;
    foreach ( const Query::RequestProperty& rp, m_requestProperties ) {
        if ( rp.optional() ) {
            s += "OPTIONAL { ";
        }

        s += QString( "?r %1 ?reqProp%2 . " )
             .arg( Soprano::Node::resourceToN3( rp.property().uri() ) )
             .arg( i++ );

        if ( rp.optional() ) {
            s += "} ";
        }
    }
    return s;
}



class Nepomuk::Query::Query::RequestProperty::Private : public QSharedData
{
public:
    Nepomuk::Types::Property m_property;
    bool m_optional;
};

Nepomuk::Query::Query::RequestProperty::RequestProperty( const Nepomuk::Types::Property& property,
                                                         bool optional )
    : d(new Private())
{
    d->m_property = property;
    d->m_optional = optional;
}

Nepomuk::Query::Query::RequestProperty::RequestProperty( const RequestProperty& other )
{
    d = other.d;
}

Nepomuk::Query::Query::RequestProperty::~RequestProperty()
{
}

Nepomuk::Query::Query::RequestProperty& Nepomuk::Query::Query::RequestProperty::operator=( const RequestProperty& other )
{
    d = other.d;
    return *this;
}

Nepomuk::Types::Property Nepomuk::Query::Query::RequestProperty::property() const
{
    return d->m_property;
}

bool Nepomuk::Query::Query::RequestProperty::optional() const
{
    return d->m_optional;
}

bool Nepomuk::Query::Query::RequestProperty::operator==( const RequestProperty& other ) const
{
    return d->m_property == other.d->m_property && d->m_optional == other.d->m_optional;
}


Nepomuk::Query::Query::Query()
    : d( new QueryPrivate() )
{
}


Nepomuk::Query::Query::Query( const Query& other )
{
    d = other.d;
}


Nepomuk::Query::Query::Query( const Term& term )
    : d ( new QueryPrivate() )
{
    d->m_term = term;
}


Nepomuk::Query::Query::~Query()
{
}


Nepomuk::Query::Query& Nepomuk::Query::Query::operator=( const Query& other )
{
    d = other.d;
    return *this;
}


Nepomuk::Query::Query& Nepomuk::Query::Query::operator=( const Term& term )
{
    d->m_term = term;
    return *this;
}


bool Nepomuk::Query::Query::isValid() const
{
    return d->m_term.isValid();
}


Nepomuk::Query::Term Nepomuk::Query::Query::term() const
{
    return d->m_term;
}


int Nepomuk::Query::Query::limit() const
{
    return d->m_limit;
}


int Nepomuk::Query::Query::offset() const
{
    return d->m_offset;
}


void Nepomuk::Query::Query::setTerm( const Term& term )
{
    d->m_term = term;
}


void Nepomuk::Query::Query::setLimit( int limit )
{
    d->m_limit = limit;
}


void Nepomuk::Query::Query::setOffset( int offset )
{
    d->m_offset = offset;
}


void Nepomuk::Query::Query::addRequestProperty( const RequestProperty& property )
{
    d->m_requestProperties.append( property );
}


void Nepomuk::Query::Query::setRequestProperties( const QList<RequestProperty>& properties )
{
    d->m_requestProperties = properties;
}


QList<Nepomuk::Query::Query::RequestProperty> Nepomuk::Query::Query::requestProperties() const
{
    return d->m_requestProperties;
}


namespace {
    template<typename T>
    bool compareQList( const QList<T>& rp1, const QList<T>& rp2 ) {
        // brute force
        foreach( const T& rp, rp1 ) {
            if ( !rp2.contains( rp ) ) {
                return false;
            }
        }
        foreach( const T& rp, rp2 ) {
            if ( !rp1.contains( rp ) ) {
                return false;
            }
        }
        return true;
    }
}

bool Nepomuk::Query::Query::operator==( const Query& other ) const
{
    return( d->m_limit == other.d->m_limit &&
            d->m_offset == other.d->m_offset &&
            d->m_term == other.d->m_term &&
            compareQList( d->m_requestProperties, other.d->m_requestProperties ) &&
            compareQList( d->m_includeFolders, other.d->m_includeFolders ) &&
            compareQList( d->m_excludeFolders, other.d->m_excludeFolders ) );
}


QString Nepomuk::Query::Query::toSparqlQuery( SparqlFlags flags ) const
{
    Term term = d->m_term;

    // restrict to files if we are a file query
    if( d->m_isFileQuery ) {
        //
        // we do not use ResourceTypeTerm since we do not want to use crappy inference every time. All files have nfo:FileDataObject type anyway
        //
        term = AndTerm( term, OrTerm( ComparisonTerm( Soprano::Vocabulary::RDF::type(), ResourceTerm(Vocabulary::NFO::FileDataObject()), ComparisonTerm::Equal ),
                                      ComparisonTerm( Soprano::Vocabulary::RDF::type(), ResourceTerm(Vocabulary::NFO::Folder()), ComparisonTerm::Equal ) ) );
    }

    // optimize whatever we can
    term = QueryPrivate::optimizeTerm( term );

    // actually build the SPARQL query string
    QueryBuilderData qbd( flags );
    QString termGraphPattern;
    if( term.isValid() ) {
        termGraphPattern = term.d_ptr->toSparqlGraphPattern( QLatin1String( "?r" ), &qbd );
        if( termGraphPattern.isEmpty() ) {
            kDebug() << "Got no valid SPARQL pattern from" << term;
            return QString();
        }
    }
    else {
        // create the "all resources query"
        termGraphPattern = QString::fromLatin1("graph ?g { ?r a ?t . } . ?g a ?gt . ?gt rdfs:subClassOf nrl:InstanceBase . ");
    }

    QString query = QString::fromLatin1( "select %1 %2 %3 %4 where { %5 %6 %7 }" )
                    .arg( flags & CreateCountQuery ? QLatin1String("count(distinct ?r)") : QLatin1String("distinct ?r"),
                          d->buildRequestPropertyVariableList(),
                          qbd.customVariables().join( QLatin1String(" ") ),
                          qbd.buildScoringExpression(),
                          termGraphPattern,
                          d->createFolderFilter( QLatin1String( "?r" ), &qbd ),
                          d->buildRequestPropertyPatterns() );

    query += qbd.buildOrderString();

    if ( d->m_offset > 0 )
        query += QString::fromLatin1( " OFFSET %1" ).arg( d->m_offset );
    if ( d->m_limit > 0 )
        query += QString::fromLatin1( " LIMIT %1" ).arg( d->m_limit );
    return query.simplified();
}


KUrl Nepomuk::Query::Query::toSearchUrl( SparqlFlags flags ) const
{
    QString sparql = toSparqlQuery( flags & ~CreateCountQuery );
    if( sparql.isEmpty() ) {
        return KUrl();
    }
    else {
        KUrl url( QLatin1String("nepomuksearch:/") );
        url.addQueryItem( QLatin1String("sparql"), sparql );
        return url;
    }
}


Nepomuk::Query::RequestPropertyMap Nepomuk::Query::Query::requestPropertyMap() const
{
    RequestPropertyMap rpm;
    for ( int i = 0; i < d->m_requestProperties.count(); ++i ) {
        rpm.insert( QString( "reqProp%1" ).arg( i+1 ), d->m_requestProperties[i].property() );
    }
    return rpm;
}


QDebug operator<<( QDebug dbg, const Nepomuk::Query::Query& query )
{
    dbg << "(Query:           " << query.term() << endl
        << " Offset:           " << query.offset()
        << " Limit:           " << query.limit() << ")";
    return dbg;
}


uint Nepomuk::Query::qHash( const Nepomuk::Query::Query& query )
{
    return qHash( query.term() );
}
