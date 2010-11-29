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
#include "optionalterm.h"
#include "queryserializer.h"
#include "queryparser.h"
#include "util.h"
#include "filequery.h"

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
#include <Soprano/Vocabulary/NAO>

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


namespace {
    Nepomuk::Query::Term prepareForSparql( const Nepomuk::Query::Term& term )
    {
        //
        // A negation is expressed via a filter. Since filters can only work on a "real" graph pattern
        // we need to make sure that such a pattern exists. This can be done by searching one in a
        // surrounding and term.
        //
        // Why is that enough?
        // Nested AndTerms are flattened before the SPARQL query is constructed in Query. Thus, an AndTerm can
        // only be embedded in an OrTerm or as a child term to either a ComparisonTerm or an OptionalTerm.
        // In both cases we need a real pattern inside the AndTerm.
        //
        // We use a type pattern for performance reasons. Thus, we assume that each resource has a type. This
        // is not perfect but much faster than using a wildcard for the property. And in the end all Nepomuk
        // resources should have a properly defined type.
        //

        switch( term.type() ) {
        case Nepomuk::Query::Term::And: {
            // check if there are negation terms without proper patterns
            Nepomuk::Query::AndTerm at = term.toAndTerm();
            Nepomuk::Query::AndTerm newAndTerm;
            bool haveNegationTerm = false;
            bool haveRealTerm = false;
            Q_FOREACH( const Nepomuk::Query::Term& term, at.subTerms() ) {
                if( term.isNegationTerm() ) {
                    haveNegationTerm = true;
                    newAndTerm.addSubTerm( term );
                }
                else {
                    if( term.isComparisonTerm() ||
                        term.isResourceTypeTerm() ||
                        term.isLiteralTerm() ) {
                        haveRealTerm = true;
                    }
                    newAndTerm.addSubTerm( prepareForSparql( term ) );
                }
            }
            if( haveNegationTerm && !haveRealTerm ) {
                newAndTerm.addSubTerm( Nepomuk::Query::ComparisonTerm( Soprano::Vocabulary::RDF::type(), Nepomuk::Query::Term() ) );
            }
            return newAndTerm;
        }

        case Nepomuk::Query::Term::Or: {
            // call prepareForSparql on all subterms
            QList<Nepomuk::Query::Term> subTerms = term.toOrTerm().subTerms();
            QList<Nepomuk::Query::Term> newSubTerms;
            Q_FOREACH( const Nepomuk::Query::Term& term, subTerms ) {
                newSubTerms.append( prepareForSparql( term ) );
            }
            return Nepomuk::Query::OrTerm( newSubTerms );
        }

        case Nepomuk::Query::Term::Negation: {
            // add an additional type term since there is no useful pattern. Otherwise
            // we would have caught it above
            return Nepomuk::Query::AndTerm(
                term,
                Nepomuk::Query::ComparisonTerm( Soprano::Vocabulary::RDF::type(), Nepomuk::Query::Term() ) );
        }

        case Nepomuk::Query::Term::Optional: {
            return Nepomuk::Query::OptionalTerm::optionalizeTerm( prepareForSparql( term.toOptionalTerm().subTerm() ) );
        }

        case Nepomuk::Query::Term::Comparison: {
            Nepomuk::Query::ComparisonTerm ct = term.toComparisonTerm();
            ct.setSubTerm( prepareForSparql( ct.subTerm() ) );
            return ct;
        }

        default:
            return term;
        }
    }


    /**
     * term is optimized and ran through prepareForSparql. The only type terms
     * we are interested in are those that are non-optional.
     */
    bool containsResourceTypeTerm( const Nepomuk::Query::Term& term )
    {
        if( term.isResourceTypeTerm() ) {
            return true;
        }

        // in an and-term a single resource type term is sufficient
        else if( term.isAndTerm() ) {
            Q_FOREACH( const Nepomuk::Query::Term& subTerm, term.toAndTerm().subTerms() ) {
                if( containsResourceTypeTerm(subTerm) ) {
                    return true;
                }
            }
        }

        // an or-term which only consists of resource type terms is valid also
        else if( term.isOrTerm() ) {
            Q_FOREACH( const Nepomuk::Query::Term& subTerm, term.toOrTerm().subTerms() ) {
                if( !containsResourceTypeTerm(subTerm) ) {
                    return false;
                }
            }
            // at this point all subterms contain a non-optional resource type term
            return true;
        }

        // fallback
        return false;
    }
}


QString Nepomuk::Query::QueryPrivate::createFolderFilter( const QString& resourceVarName, QueryBuilderData* qbd ) const
{
    if ( m_includeFolders.count() + m_excludeFolders.count() ) {
        QString uriVarName = qbd->uniqueVarName();
        QString filter = resourceVarName + ' ' + Soprano::Node::resourceToN3( Nepomuk::Vocabulary::NIE::url() ) + ' ' + uriVarName + QLatin1String( " . " );
        if ( !m_includeFolders.isEmpty() ) {
            QStringList includeFilter;
            for( QHash<KUrl, bool>::ConstIterator it = m_includeFolders.constBegin();
                it != m_includeFolders.constEnd(); ++it ) {
                const QString urlStr = it.key().url(KUrl::AddTrailingSlash);
                if( it.value() )
                    includeFilter.append( QString::fromLatin1("(^%1)").arg( urlStr ) );
                else
                    includeFilter.append( QString::fromLatin1("(^%1[^/]*$)").arg( urlStr ) );
            }
            filter += QString::fromLatin1( " FILTER(REGEX(STR(%1), \"%2\")) ." ).arg( uriVarName ).arg( includeFilter.join( "|" ) );
        }
        if ( !m_excludeFolders.isEmpty() ) {
            filter += QString::fromLatin1( " FILTER(!REGEX(STR(%1), \"^(%2)\")) ." ).arg( uriVarName ).arg( m_excludeFolders.toStringList(KUrl::AddTrailingSlash).join( "|" ) );
        }
        return filter;
    }
    else {
        return QString();
    }
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
    return( d->m_term.isValid() || isFileQuery() );
}


bool Nepomuk::Query::Query::isFileQuery() const
{
    return d->m_isFileQuery;
}


Nepomuk::Query::FileQuery Nepomuk::Query::Query::toFileQuery() const
{
    return FileQuery( *this );
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


void Nepomuk::Query::Query::setFullTextScoringEnabled( bool enabled )
{
    d->m_fullTextScoringEnabled = enabled;
}


void Nepomuk::Query::Query::setFullTextScoringSortOrder( Qt::SortOrder order )
{
    d->m_fullTextScoringSortOrder = order;
}


bool Nepomuk::Query::Query::fullTextScoringEnabled() const
{
    return d->m_fullTextScoringEnabled;
}


Qt::SortOrder Nepomuk::Query::Query::fullTextScoringSortOrder() const
{
    return d->m_fullTextScoringSortOrder;
}


void Nepomuk::Query::Query::setQueryFlags( QueryFlags flags )
{
    d->m_flags = flags;
}


Nepomuk::Query::Query::QueryFlags Nepomuk::Query::Query::queryFlags() const
{
    return d->m_flags;
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


bool Nepomuk::Query::Query::operator==( const Query& other ) const
{
    return( d->m_limit == other.d->m_limit &&
            d->m_offset == other.d->m_offset &&
            d->m_term == other.d->m_term &&
            compareQList( d->m_requestProperties, other.d->m_requestProperties ) &&
            compareHash( d->m_includeFolders, other.d->m_includeFolders ) &&
            compareQList( d->m_excludeFolders, other.d->m_excludeFolders ) &&
            d->m_isFileQuery == other.d->m_isFileQuery &&
            d->m_fileMode == other.d->m_fileMode );
}


bool Nepomuk::Query::Query::operator!=( const Query& other ) const
{
    return( d->m_limit != other.d->m_limit ||
            d->m_offset != other.d->m_offset ||
            d->m_term != other.d->m_term ||
            !compareQList( d->m_requestProperties, other.d->m_requestProperties ) ||
            !compareHash( d->m_includeFolders, other.d->m_includeFolders ) ||
            !compareQList( d->m_excludeFolders, other.d->m_excludeFolders ) ||
            d->m_isFileQuery != other.d->m_isFileQuery ||
            d->m_fileMode != other.d->m_fileMode );
}


QString Nepomuk::Query::Query::toSparqlQuery( SparqlFlags sparqlFlags ) const
{
    Term term = d->m_term;

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


    // convert request properties into ComparisonTerms
    // in ask and count query mode we can omit the optional req props
    for ( int i = 0; i < d->m_requestProperties.count(); ++i ) {
        const RequestProperty& rp = d->m_requestProperties[i];
        ComparisonTerm rpt( rp.property(), Term() );
        rpt.setVariableName( QString::fromLatin1("reqProp%1").arg(i+1) );
        if( rp.optional() && !( sparqlFlags&(CreateAskQuery|CreateCountQuery) ) ) {
            term = term && OptionalTerm::optionalizeTerm( rpt );
        }
        else if( !rp.optional() ) {
            term = term && rpt;
        }
    }


    // optimize whatever we can
    term = prepareForSparql( term ).optimized();

    // actually build the SPARQL query patterns
    QueryBuilderData qbd( d.constData(), sparqlFlags );
    QString termGraphPattern;
    if( term.isValid() ) {
        termGraphPattern = term.d_ptr->toSparqlGraphPattern( QLatin1String( "?r" ), &qbd );
        if( termGraphPattern.isEmpty() ) {
            kDebug() << "Got no valid SPARQL pattern from" << term;
            return QString();
        }
    }

    // build the list of variables to select (in addition to the main result variable ?r)
    QStringList selectVariables = qbd.customVariables();

    // add additional scoring variable if requested
    if( d->m_fullTextScoringEnabled ) {
        const QString scoringExpression = qbd.buildScoringExpression();
        if( !scoringExpression.isEmpty() )
            selectVariables << scoringExpression;
    }

    // restrict to resources to user visible types only. There is no need to do that for file queries
    // as those already restrict the type.
    // Since we do the restriction on the type it gets useless as soon as the query contains a non-optional
    // type term itself
    QString userVisibilityRestriction;
    if( !d->m_isFileQuery && !containsResourceTypeTerm(term) && !(queryFlags()&NoResultRestrictions) ) {
        userVisibilityRestriction = QString::fromLatin1( "?r a %1 . %1 %2 %3 . " )
                                    .arg( qbd.uniqueVarName(),
                                          Soprano::Node::resourceToN3(Soprano::Vocabulary::NAO::userVisible()),
                                          Soprano::Node::literalToN3(true) );
    }

    // build the core of the query - the part that never changes
    QString queryBase = QString::fromLatin1( "where { %1 %2 %3 }" )
                        .arg( termGraphPattern,
                              d->createFolderFilter( QLatin1String( "?r" ), &qbd ),
                              userVisibilityRestriction );

    // build the final query
    QString query;
    if( sparqlFlags & CreateCountQuery ) {
        if( selectVariables.isEmpty() ) {
            // when there are no additional variables we can perfectly use count(distinct)
            query = QString::fromLatin1("select count(distinct ?r) as ?cnt %1 %2")
                    .arg( selectVariables.join( QLatin1String(" " ) ),
                          queryBase );
        }
        else {
            // when there are additional variables we need to do some magic to count the
            // number of rows instead of a list of counts
            // we cannot simply leave out the additional variables since that would change
            // the number of results.
            query = QString::fromLatin1("select count(%1) as ?cnt %2 where { { select count(*) as %1 ?r %3 } }")
                    .arg(qbd.uniqueVarName(),
                         selectVariables.join( QLatin1String(" " ) ),
                         queryBase );
        }
    }
    else if( sparqlFlags & CreateAskQuery ) {
        query = QLatin1String( "ask ") + queryBase;
    }
    else {
        QString fullTextExcerptExpression;
        if( !(queryFlags()&WithoutFullTextExcerpt) ) {
            fullTextExcerptExpression = qbd.buildSearchExcerptExpression();
        }
        query = QString::fromLatin1( "select distinct ?r %1 %2 %3" )
                .arg( selectVariables.join( QLatin1String(" " ) ),
                      fullTextExcerptExpression,
                      queryBase );
        query += qbd.buildOrderString();
    }

    // offset and limit
    if ( d->m_offset > 0 )
        query += QString::fromLatin1( " OFFSET %1" ).arg( d->m_offset );
    if ( d->m_limit > 0 )
        query += QString::fromLatin1( " LIMIT %1" ).arg( d->m_limit );

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
    // the nepomuksearch:/ KIO slave does not handle count or ask queries
    flags &= ~CreateCountQuery;
    flags &= ~CreateAskQuery;

    Query q( *this );

    // the nepomuksearch:/ KIO slave does not make use of full text scores. Thus, we avoid the
    // overhead by disabling them
    q.setFullTextScoringEnabled( false );

    KUrl url( QLatin1String("nepomuksearch:/") );
    if( flags == NoFlags )
        url.addQueryItem( QLatin1String("encodedquery"), q.toString() );
    else
        url.addQueryItem( QLatin1String("sparql"), q.toSparqlQuery( flags ) );

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


Nepomuk::Query::Query Nepomuk::Query::Query::optimized() const
{
    Query newQuery( *this );
    newQuery.setTerm( term().optimized() );
    return newQuery;
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
