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
#include "queryserializer.h"
#include "queryparser.h"

#include <QtCore/QDebug>
#include <QtCore/QDateTime>
#include <QtCore/QRegExp>
#include <QtCore/QVariant>
#include <QtCore/QTextStream>

#include <Soprano/Node>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/NRL>

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



QStringList Nepomuk::Query::QueryPrivate::buildRequestPropertyVariableList() const
{
    QStringList s;
    for ( int i = 1; i <= m_requestProperties.count(); ++i ) {
        s << QString( "?reqProp%1 " ).arg( i );
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


bool Nepomuk::Query::Query::isFileQuery() const
{
    return d->m_isFileQuery;
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
            compareQList( d->m_excludeFolders, other.d->m_excludeFolders ) &&
            d->m_isFileQuery == other.d->m_isFileQuery &&
            d->m_fileMode == other.d->m_fileMode );
}


QString Nepomuk::Query::Query::toSparqlQuery( SparqlFlags flags ) const
{
    Term term = d->m_term;

    // we do not need scores when counting results
    if( flags & CreateCountQuery )
        flags |= WithoutScoring;

    // restrict to files if we are a file query
    if( d->m_isFileQuery ) {
        //
        // we do not use ResourceTypeTerm since we do not want to use crappy inference every time. All files have nfo:FileDataObject type anyway
        //
        Term fileModeTerm;
        ComparisonTerm fileTerm( Soprano::Vocabulary::RDF::type(), ResourceTerm(Vocabulary::NFO::FileDataObject()), ComparisonTerm::Equal );
        ComparisonTerm folderTerm( Soprano::Vocabulary::RDF::type(), ResourceTerm(Vocabulary::NFO::Folder()), ComparisonTerm::Equal );
        if( d->m_fileMode == FileQuery::QueryFiles )
            fileModeTerm = AndTerm( fileTerm, NegationTerm::negateTerm( folderTerm ) );
        else if( d->m_fileMode == FileQuery::QueryFolders )
            fileModeTerm = AndTerm( folderTerm, NegationTerm::negateTerm( fileTerm ) );
        else
            fileModeTerm = OrTerm( fileTerm, folderTerm );
        term = AndTerm( term, fileModeTerm );
    }

    // optimize whatever we can
    term = QueryPrivate::optimizeTerm( term );

    // actually build the SPARQL query patterns
    QueryBuilderData qbd( flags );
    QString termGraphPattern;
    if( term.isValid() ) {
        termGraphPattern = term.d_ptr->toSparqlGraphPattern( QLatin1String( "?r" ), &qbd );
        if( termGraphPattern.isEmpty() ) {
            kDebug() << "Got no valid SPARQL pattern from" << term;
            return QString();
        }
    }

    // build the list of variables to select (in addition to the main result variable ?r)
    QStringList selectVariables = d->buildRequestPropertyVariableList() + qbd.customVariables();
    const QString scoringExpression = qbd.buildScoringExpression();
    if( !scoringExpression.isEmpty() )
        selectVariables << scoringExpression;

    // restrict to resources from nrl:InstanceBases only. There is no need to do that for file queries
    // as those alreday restrict the type. While in theory there might be file or folder resources in
    // non-InstanceBase graphs it is very unlikely and omitting the instanceBaseRestriction will give
    // us a performance gain - even if small it is worth it.
    const QString instanceBaseRestriction =
        QString::fromLatin1( "graph %1 { ?r a %2 . } . { %1 a %3 . } UNION { %1 a %4 . } . " )
        .arg( qbd.uniqueVarName(),
              qbd.uniqueVarName(),
              Soprano::Node::resourceToN3(Soprano::Vocabulary::NRL::InstanceBase()),
              Soprano::Node::resourceToN3(Soprano::Vocabulary::NRL::DiscardableInstanceBase()) );

    // build the core of the query - the part that never changes
    QString queryBase = QString::fromLatin1( "%1 where { %2 %3 %4 %5 }" )
                        .arg( selectVariables.join( QLatin1String(" " ) ),
                              termGraphPattern,
                              d->createFolderFilter( QLatin1String( "?r" ), &qbd ),
                              d->buildRequestPropertyPatterns(),
                              d->m_isFileQuery ? QString() : instanceBaseRestriction );

    // add optional order terms
    queryBase += qbd.buildOrderString();

    // offset and limit
    if ( d->m_offset > 0 )
        queryBase += QString::fromLatin1( " OFFSET %1" ).arg( d->m_offset );
    if ( d->m_limit > 0 )
        queryBase += QString::fromLatin1( " LIMIT %1" ).arg( d->m_limit );

    // build the final query
    QString query;
    if( flags & CreateCountQuery ) {
        if( selectVariables.isEmpty() ) {
            // when there are no additional variables we can perfectly use count(distinct)
            query = QLatin1String("select count(distinct ?r) as ?cnt ") + queryBase;
        }
        else {
            // when there are additional variables we need to do some magic to count the
            // number of rows instead of a list of counts
            query = QString::fromLatin1("select count(%1) as ?cnt where { { select count(*) as %1 ?r %2 } }")
                    .arg(qbd.uniqueVarName(), queryBase );
        }
    }
    else {
        query = QLatin1String( "select distinct ?r " ) + queryBase;
    }

    return query.simplified();
}


KUrl Nepomuk::Query::Query::toSearchUrl( SparqlFlags flags ) const
{
    return toSearchUrl( QString(), flags );
}


// This is a bit dodgy: if there are sparql flags we encode the SPARQL query into the url
// otherwise the serialized query (which allows for more power in the kio slave). It would
// probably be nicer to somehow put the flags in the URL. But new query items in the URL
// would make the URL handling in the kio slave more complicated.... oh, well.
KUrl Nepomuk::Query::Query::toSearchUrl( const QString& customTitle, SparqlFlags flags ) const
{
    flags &= ~CreateCountQuery;
    KUrl url( QLatin1String("nepomuksearch:/") );
    if( flags == NoFlags )
        url.addQueryItem( QLatin1String("encodedquery"), toString() );
    else
        url.addQueryItem( QLatin1String("sparql"), toSparqlQuery( flags ) );
    if( !customTitle.isEmpty() )
        url.addPath( customTitle );
    else
        url.addPath( titleFromQueryUrl( url ) );
    return url;
}


Nepomuk::Query::RequestPropertyMap Nepomuk::Query::Query::requestPropertyMap() const
{
    RequestPropertyMap rpm;
    for ( int i = 0; i < d->m_requestProperties.count(); ++i ) {
        rpm.insert( QString( "reqProp%1" ).arg( i+1 ), d->m_requestProperties[i].property() );
    }
    return rpm;
}

QString Nepomuk::Query::Query::toString() const
{
    return Nepomuk::Query::serializeQuery( *this );
}


// static
Nepomuk::Query::Query Nepomuk::Query::Query::fromString( const QString& queryString )
{
    return Nepomuk::Query::parseQuery( queryString );
}


namespace {
    /**
     * Returns an empty string for sparql query URLs.
     */
    inline QString extractPlainQuery( const KUrl& url ) {
        if( url.queryItems().contains( "query" ) ) {
            return url.queryItem( "query" );
        }
        else if ( !url.hasQuery() ) {
            return url.path().section( '/', 0, 0, QString::SectionSkipEmpty );
        }
        else {
            return QString();
        }
    }
}

// static
Nepomuk::Query::Query Nepomuk::Query::Query::fromQueryUrl( const KUrl& url )
{
    if( url.protocol() != QLatin1String("nepomuksearch") ) {
        kDebug() << "No nepomuksearch:/ URL:" << url;
        return Query();
    }

    if ( url.queryItems().contains( "sparql" ) ) {
        kDebug() << "Cannot parse SPARQL query from:" << url;
        return Query();
    }
    else if( url.queryItems().contains( "encodedquery" ) ) {
        return fromString( url.queryItem( "encodedquery") );
    }
    else {
        Query query = QueryParser::parseQuery( extractPlainQuery(url) );
        query.setRequestProperties( QList<RequestProperty>() << Nepomuk::Query::Query::RequestProperty( Nepomuk::Vocabulary::NIE::url(), true ) );
        return query;
    }
}


// static
QString Nepomuk::Query::Query::sparqlFromQueryUrl( const KUrl& url )
{
    if( url.protocol() != QLatin1String("nepomuksearch") ) {
        kDebug() << "No nepomuksearch:/ URL:" << url;
        return QString();
    }

    if( url.queryItems().contains( "sparql" ) ) {
        return url.queryItem( "sparql" );
    }
    else {
        Query query = fromQueryUrl( url );
        if( query.isValid() ) {
            query.setRequestProperties( QList<RequestProperty>() << Nepomuk::Query::Query::RequestProperty( Nepomuk::Vocabulary::NIE::url(), true ) );
            return query.toSparqlQuery();
        }
        else {
            return QString();
        }
    }
}


// static
QString Nepomuk::Query::Query::titleFromQueryUrl( const KUrl& url )
{
    if( url.protocol() != QLatin1String("nepomuksearch") ) {
        kDebug() << "No nepomuksearch:/ URL:" << url;
        return QString();
    }

    // the title is the first section of the path, but only if we have a query
    if( url.hasQuery() ) {
        QString title = url.path().section( '/', 0, 0, QString::SectionSkipEmpty );
        if(!title.isEmpty())
            return title;
    }

    // no title in the path, try to get the user query, i.e. a non-encoded and non-sparql query
    QString userQuery = extractPlainQuery( url );
    if ( !userQuery.isEmpty() ) {
        return i18nc( "@title UDS_DISPLAY_NAME for a KIO directory listing. %1 is the query the user entered.",
                      "Query Results from '%1'",
                      userQuery );
    }

    // fallback: generic query title
    return i18nc( "@title UDS_DISPLAY_NAME for a KIO directory listing.",
                  "Query Results");
}


Nepomuk::Query::Query Nepomuk::Query::operator&&( const Query& query, const Term& term )
{
    Query newQuery( query );
    newQuery.setTerm( query.term() && term );
    return newQuery;
}


Nepomuk::Query::Query Nepomuk::Query::operator||( const Query& query, const Term& term )
{
    Query newQuery( query );
    newQuery.setTerm( query.term() || term );
    return newQuery;
}


Nepomuk::Query::Query Nepomuk::Query::operator!( const Query& query )
{
    Query newQuery( query );
    newQuery.setTerm( !query.term() );
    return newQuery;
}


QDebug operator<<( QDebug dbg, const Nepomuk::Query::Query& query )
{
    return dbg << query.toString();
}


uint Nepomuk::Query::qHash( const Nepomuk::Query::Query& query )
{
    return qHash( query.term() );
}
