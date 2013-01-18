/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007-2011 Sebastian Trueg <trueg@kde.org>

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

#include "queryparser.h"
#include "query.h"
#include "query_p.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"
#include "comparisonterm.h"
#include "dateparser_p.h"
#include "nfo.h"

#include <QtCore/QRegExp>
#include <QtCore/QSet>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <kdebug.h>
#include <klocale.h>

#include "resourcemanager.h"
#include "property.h"
#include "literal.h"

#include <Soprano/Node>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>


using namespace Nepomuk::Query;

namespace {
    Nepomuk::Query::ComparisonTerm::Comparator fieldTypeRelationFromString( const QString& s ) {
        if ( s == "=" ) {
            return Nepomuk::Query::ComparisonTerm::Equal;
        }
        else if ( s == ":" ) {
            return Nepomuk::Query::ComparisonTerm::Contains;
        }
        else if ( s == ">" ) {
            return Nepomuk::Query::ComparisonTerm::Greater;
        }
        else if ( s == "<" ) {
            return Nepomuk::Query::ComparisonTerm::Smaller;
        }
        else if ( s == ">=" ) {
            return Nepomuk::Query::ComparisonTerm::GreaterOrEqual;
        }
        else if ( s == "<=" ) {
            return Nepomuk::Query::ComparisonTerm::SmallerOrEqual;
        }
        else {
            kDebug() << "FIXME: Unsupported relation:" << s;
            return Nepomuk::Query::ComparisonTerm::Equal;
        }
    }

    QString stripQuotes( const QString& s, bool* hadQuotes = 0 ) {
        if ( s[0] == '\'' ||
             s[0] == '\"' ) {
            if( hadQuotes )
                *hadQuotes = true;
            return s.mid( 1 ).left( s.length()-2 );
        }
        else {
            if( hadQuotes )
                *hadQuotes = false;
            return s;
        }
    }

    QUrl tryToBeIntelligentAboutParsingUrl( const QString& s ) {
        if ( s.contains( '%' ) && !s.contains( '/' ) ) {
            return QUrl::fromEncoded( s.toLatin1() );
        }
        else {
            return QUrl( s );
        }
    }

    Soprano::LiteralValue createLiteral( const QString& s, bool globbing ) {
        // 1. check if it is a number
        QString clearString(s);
        clearString.remove(QLatin1Char('\''));
        clearString.remove(QLatin1Char('"'));
        bool b = false;
        int i = clearString.toInt( &b );
        if ( b )
            return Soprano::LiteralValue( i );
        double d = clearString.toDouble( &b );
        if ( b )
            return Soprano::LiteralValue( d );

        // 2. no number - continue with the original string

        // no globbing if we have quotes or if there already is a wildcard
        if ( s[0] == QLatin1Char('\'') ||
             s[0] == QLatin1Char('\"') ) {
            return s;
        }

        //
        // we can only do query term globbing for strings longer than 3 chars
        //
        if( globbing && s.length() > 3 && !s.endsWith('*') && !s.endsWith('?') )
            return QString(s + '*');
        else
            return s;
    }

    bool positiveTerm( const QString& s_ ) {
        const QString s = s_.toLower().simplified();
        if(s.isEmpty())
            return true;
        else if(s == "+")
            return true;
        else if(s == "-")
            return false;
        else if(s == "!")
            return false;
        else if(s == "not")
            return false;
        else //unrecognized capture
            return true;
    }

    // A filename pattern needs to contain one dot and at least one '*' or '?':
    // *.mp3
    // hello?.txt
    // hello?.*
    // test*.???
    bool isFilenamePattern( const QString& s )
    {
        return( !s.contains(' ') &&
                s.count('.') == 1 &&
                s.count('*') + s.count('?') > 0 );
    }

    Nepomuk::Query::ComparisonTerm createFilenamePatternTerm( const QString& s )
    {
        QString regex = QRegExp::escape(s);
        regex.replace( "\\*", QLatin1String( ".*" ) );
        regex.replace( "\\?", QLatin1String( "." ) );
        regex.replace("\\", "\\\\");
        regex.prepend('^');
        regex.append('$');
        return Nepomuk::Query::ComparisonTerm( Nepomuk::Vocabulary::NFO::fileName(),
                                               Nepomuk::Query::LiteralTerm( regex ),
                                               Nepomuk::Query::ComparisonTerm::Regexp );
    }

    /**
     * Merging literal terms is an optimization which is based on the assumption that most
     * users want to search for the full text terms they enter in the value of the same
     * property.
     * Since merging two literals "foo" and "bar" into one term "foo AND bar" effectively
     * changes the result set (the former allows that "foo" occurs in a property value
     * different from "bar" while the latter forces them to occur in the same.)
     * But the resulting query is much faster.
     */
    Nepomuk::Query::Term mergeLiteralTerms( const Nepomuk::Query::Term& term )
    {
        if( term.isAndTerm() ) {
            AndTerm mergedTerm;
            QStringList fullTextTerms;
            Q_FOREACH( const Term& st, term.toAndTerm().subTerms() ) {
                if( st.isLiteralTerm() ) {
                    fullTextTerms << st.toLiteralTerm().value().toString();
                }
                else {
                    mergedTerm.addSubTerm( st );
                }
            }
            mergedTerm.addSubTerm( LiteralTerm( fullTextTerms.join( QString::fromLatin1(" AND ") ) ) );
            return mergedTerm.optimized();
        }
        else {
            return term;
        }
    }

    /**
     * Tries to setup the sub-term of the given comparisonterm according to its property.
     * This can fail if the sub-term cannot be converted to the property's range.
     *
     * This is used in QueryParser::Private::resolveFields
     */
    bool setupComparisonTermSubTerm(Nepomuk::Query::ComparisonTerm& ct)
    {
        // property with resource range
        if(ct.property().range().isValid()) {
            if(ct.subTerm().isLiteralTerm()) {
                // here we need a string. everything else does not work since we match labels
                return ct.subTerm().toLiteralTerm().value().isString();
            }
            else {
                return true;
            }
        }

        // property with literal range
        else if(ct.subTerm().isLiteralTerm()) {
            // only strings can be matched via bif:contains
            if(ct.comparator() == Nepomuk::Query::ComparisonTerm::Contains &&
               ct.property().literalRangeType().dataType() != QVariant::String) {
                ct.setComparator(Nepomuk::Query::ComparisonTerm::Equal);
            }

            // try to convert the value via QVariant
            QVariant v = ct.subTerm().toLiteralTerm().value().variant();
            if(v.convert(ct.property().literalRangeType().dataType())) {
                ct.setSubTerm(Nepomuk::Query::LiteralTerm(v));
                return true;
            }

            // try some heuristics
            else {
                if(ct.property().literalRangeType().dataType() == QVariant::DateTime) {
                    // if it is an int with 4 digits it is very likely a year
                    if(ct.subTerm().toLiteralTerm().value().isInt() ) {
                        const int year = ct.subTerm().toLiteralTerm().value().toInt();
                        if( year >= 1000 && year <= 9999) {
                            QDate date(year, 1, 1);
                            QTime time(0,0);
                            // greater than last day in same year
                            // greater or equal than the first day in same year
                            // smaller than first day in same year
                            // smaller or equal than last day in same year
                            if(ct.comparator() == ComparisonTerm::Greater ||
                               ct.comparator() == ComparisonTerm::SmallerOrEqual) {
                                date.setYMD(date.year(), 12, 31);
                                time.setHMS(23,59,59,999);
                            }
                            ct.setSubTerm(Nepomuk::Query::LiteralTerm(QDateTime(date, time)));
                            return true;
                        }
                    }
                }
            }
        }

        // fallback
        return false;
    }

#ifndef Q_CC_MSVC
#warning Make the parser handle different data, time, and datetime encodings as well as suffixes like MB or GB
#endif
#if 0
    QDateTime parseDateTime( const Soprano::LiteralValue& literal )
    {
        //TODO: change to DateTime parser once complete
        Nepomuk::Search::DateParser date( literal.toString() );
        if( date.hasDate() ) {
            return QDateTime( date.getDate() );
        }
        else {
            Nepomuk::Search::TimeParser time( literal.toString() );
            if(time.hasTime() )
                return QDateTime(QDate::currentDate(), time.next() );
            else
                return QDateTime(); //return invalid datetime
        }
    }


    Soprano::LiteralValue parseSizeType( const Soprano::LiteralValue& literal )
    {
        const double KiB = 1024.0;
        const double MiB = KiB * 1024.0;
        const double GiB = MiB * 1024.0;
        const double TiB = GiB * 1024.0;

        const double KB = 1000.0;
        const double MB = KB * 1000.0;
        const double GB = MB * 1000.0;
        const double TB = GB * 1000.0;

        QHash<QString, double> sizes;
        sizes.insert( "KiB", KiB );
        sizes.insert( "MiB", MiB );
        sizes.insert( "GiB", GiB );
        sizes.insert( "TiB", TiB );
        sizes.insert( "KB", KB );
        sizes.insert( "MB", MB );
        sizes.insert( "GB", GB );
        sizes.insert ("TB", TB );

        for ( QHash<QString, double>::const_iterator i = sizes.constBegin();
              i != sizes.constEnd(); ++i ) {
            QRegExp cur( QString("^([\\d]+.?[\\d]*)[\\s]*%1$").arg( i.key() ) );
            if( cur.indexIn( literal.toString() ) != -1 ) {
                double value = cur.cap( 1 ).toDouble();
                double newValue = value * i.value();
                kDebug() << "Found value" << value << i.key() << "->" << newValue;
                return Soprano::LiteralValue( newValue );
            }
        }
        return literal;
    }


    Nepomuk::Query::Term resolveLiteralValues( const Nepomuk::Query::Term& term )
    {
        switch( term.type() ) {
        case Nepomuk::Query::Term::Comparison: {
            Nepomuk::Query::ComparisonTerm cterm = term.toComparisonTerm();
            Nepomuk::Types::Property p( cterm.property() );
            if ( p.literalRangeType().isValid() ) {
                Q_ASSERT( cterm.subTerm().isLiteralTerm() );

                Nepomuk::Query::ComparisonTerm newTerm;
                newTerm.setComparator( cterm.comparator() );
                newTerm.setProperty( QUrl(cterm.property()) );

                // now try to resolve the literal
                const Nepomuk::Query::LiteralTerm subTerm = cterm.subTerm().toLiteralTerm();
                const Nepomuk::Types::Literal lt = p.literalRangeType();
                if ( lt.dataType() == QVariant::DateTime &&
                     !subTerm.value().isDateTime() ) {
                    QDateTime dateTime = parseDateTime( subTerm.value() );
                    if ( dateTime.isValid() ) {
                        newTerm.setSubTerm( Nepomuk::Query::LiteralTerm( dateTime ) );
                        return newTerm;
                    }
                }
                else if ( lt.dataType() == QVariant::Int &&
                          !subTerm.value().isInt() ) {
                    newTerm.setSubTerm( Nepomuk::Query::LiteralTerm( parseSizeType( subTerm.value() ) ) );
                    return newTerm;
                }
            }

            return term;
        }

        case Nepomuk::Query::Term::And:
        case Nepomuk::Query::Term::Or: {
            QList<Nepomuk::Query::Term> newSubTerms;
            foreach( const Nepomuk::Query::Term& t, static_cast<const Nepomuk::Query::GroupTerm&>( term ).subTerms() ) {
                newSubTerms << resolveLiteralValues(t);
            }
            if ( term.isAndTerm() )
                return Nepomuk::Query::AndTerm( newSubTerms );
            else
                return Nepomuk::Query::OrTerm( newSubTerms );
        }

        default:
            return term;
        }
    }
#endif

    // a field differs from a plain term in that it does never allow comparators
    const char* s_fieldNamePattern = "([^\\s\"':=<>]+|(?:([\"'])[^\"':=<>]+\\%1))";
    const char* s_plainTermPattern = "([^-][^\\s\"':=<>]*|(?:([\"'])[^\"']+\\%1))";
    const char* s_inExclusionPattern = "((?:[\\+\\-\\!]\\s*|[nN][oO][tT]\\s+)?)";
    const char* s_uriPattern = "<([^<>]+)>";
    const char* s_comparatorPattern = "(:|\\<=|\\>=|=|\\<|\\>)";

    /**
     * Creating QRegExp is expensive, copying them is cheap. Thus, we keep static
     * instances of the regexps around which we only have to create once.
     */
    class QueryParserRegExpPool
    {
    public:
        QueryParserRegExpPool()
            : plainTermRx( QLatin1String(s_inExclusionPattern)
                  + QString::fromLatin1(s_plainTermPattern).arg( 3 ) ),
              fieldRx( QLatin1String(s_inExclusionPattern)
                  + QString::fromLatin1(s_fieldNamePattern).arg( 3 )
                  + QLatin1String(s_comparatorPattern)
                  + QString::fromLatin1(s_plainTermPattern).arg( 6 ) ),
              propertyRx( QLatin1String(s_inExclusionPattern)
                  + QLatin1String(s_uriPattern)
                  + QLatin1String(s_comparatorPattern)
                  + QString::fromLatin1(s_plainTermPattern).arg( 5 ) ),
              resourceRx( QLatin1String(s_inExclusionPattern)
                  + QLatin1String(s_uriPattern)
                  + QLatin1String("(?::|=)")
                  + QLatin1String(s_uriPattern) ),
              fieldFieldRx( QLatin1String(s_inExclusionPattern)
                  + QString::fromLatin1(s_fieldNamePattern).arg( 3 )
                  + QLatin1String(s_comparatorPattern)
                  + QLatin1String("\\(")
                  +  QString::fromLatin1(s_fieldNamePattern).arg( 6 )
                  + QLatin1String(s_comparatorPattern)
                  + QString::fromLatin1(s_plainTermPattern).arg( 9 )
                  + QLatin1String("\\)") )
        {
        }

        // match a simple search text
        // captures: 1 - The optional + or - sign (may be empty)
        //           2 - the search text (including optional paranthesis)
        QRegExp plainTermRx;

        // match a field search term: fieldname + relation (:, =, etc) + search text with optional paranthesis
        // captures: 1 - The optional + or - sign (may be empty)
        //           2 - fieldname
        //           3 - relation
        //           4 - search text (including optional paranthesis)
        QRegExp fieldRx;

        // match a property URI search term: property URI + relation (:, =, etc) + search text with optional paranthesis
        // captures: 1 - The optional + or - sign (may be empty)
        //           2 - property URI
        //           3 - relation
        //           4 - search text (including optional paranthesis)
        QRegExp propertyRx;

        // match a property URI search term: property URI + relation (:, =, etc) + resource URI
        // captures: 1 - The optional + or - sign (may be empty)
        //           2 - property URI
        //           3 - resource URI
        QRegExp resourceRx;

        QRegExp fieldFieldRx;
    };

    // the one global instance used for the statis QueryParser methods
    K_GLOBAL_STATIC( QueryParserRegExpPool, s_regExpPool )
}


class Nepomuk::Query::QueryParser::Private
{
public:
    QueryParser* q;

    QSet<QString> andKeywords;
    QSet<QString> orKeywords;
    mutable QHash<QString, QList<Types::Property> > fieldMatchCache;
    QMutex fieldMatchCacheMutex;

    /// set by resolveFields if the query is in fact invalid
    bool m_invalidQuery;

    /**
     * Resolve the fields in all ComparisonTerms by looking up possible matching
     * properties.
     * This method will set m_invalidQuery in case there is one term for which
     * no property can be matched.
     */
    Nepomuk::Query::Term resolveFields( const Nepomuk::Query::Term& term );
};



Term QueryParser::Private::resolveFields(const Term &term)
{
    switch( term.type() ) {
    case Nepomuk::Query::Term::And:
    case Nepomuk::Query::Term::Or: {
        QList<Nepomuk::Query::Term> newSubTerms;
        foreach( const Nepomuk::Query::Term& t, static_cast<const Nepomuk::Query::GroupTerm&>( term ).subTerms() ) {
            Nepomuk::Query::Term resolvedTerm = resolveFields(t);
            if ( resolvedTerm.isValid() )
                newSubTerms << resolvedTerm;
            else
                return Nepomuk::Query::Term();
        }
        if ( term.isAndTerm() )
            return Nepomuk::Query::AndTerm( newSubTerms );
        else
            return Nepomuk::Query::OrTerm( newSubTerms );
    }


    case Nepomuk::Query::Term::Negation: {
        return Nepomuk::Query::NegationTerm::negateTerm( resolveFields( term.toNegationTerm().subTerm() ) );
    }


    case Nepomuk::Query::Term::Comparison: {
        Nepomuk::Query::ComparisonTerm newTerm;
        newTerm.setComparator( term.toComparisonTerm().comparator() );
        newTerm.setProperty( term.toComparisonTerm().property() );
        newTerm.setSubTerm( resolveFields( term.toComparisonTerm().subTerm() ) );

        // A very dumb test to see if the property is set or not: does the URI have a scheme.
        // With a proper parser and in-place property matching there will be no need for this anymore
        if ( newTerm.property().uri().scheme().isEmpty() ) {
            QList<Nepomuk::Types::Property> properties = q->matchProperty( term.toComparisonTerm().property().uri().toString() );
            // we only use a max of 4 properties, otherwise the queries get too big
            // in addition we try to exclude properties which do not make sense:
            // - numerical values can never match a resource
            Nepomuk::Query::OrTerm orTerm;
            for( int i = 0; i < properties.count() && orTerm.subTerms().count() < 4; ++i ) {
                const Nepomuk::Types::Property& property = properties[i];
                Nepomuk::Query::ComparisonTerm t( newTerm );
                t.setProperty( property );
                if(setupComparisonTermSubTerm(t)) {
                    orTerm.addSubTerm( t );
                }
            }
            if(!orTerm.isValid()) {
                m_invalidQuery = true;
                return Nepomuk::Query::Term();
            }
            return orTerm.optimized();
        }
    }

    default:
        return term;
    }
}


Nepomuk::Query::QueryParser::QueryParser()
    : d( new Private() )
{
    d->q = this;

    QString andListStr = i18nc( "Boolean AND keyword in desktop search strings. "
                                "You can add several variants separated by spaces, "
                                "e.g. retain the English one alongside the translation; "
                                "keywords are not case sensitive. Make sure there is "
                                "no conflict with the OR keyword.",
                                "and" );
    foreach ( const QString &andKeyword, andListStr.split( ' ', QString::SkipEmptyParts ) ) {
        d->andKeywords.insert( andKeyword.toLower() );
    }
    QString orListStr = i18nc( "Boolean OR keyword in desktop search strings. "
                               "You can add several variants separated by spaces, "
                               "e.g. retain the English one alongside the translation; "
                               "keywords are not case sensitive. Make sure there is "
                               "no conflict with the AND keyword.",
                               "or" );
    foreach ( const QString &orKeyword, orListStr.split( ' ', QString::SkipEmptyParts ) ) {
        d->orKeywords.insert( orKeyword.toLower() );
    }
}


Nepomuk::Query::QueryParser::~QueryParser()
{
    delete d;
}


QList<Nepomuk::Types::Property> Nepomuk::Query::QueryParser::matchProperty( const QString& fieldName ) const
{
    kDebug() << fieldName;

    QMutexLocker lock( &d->fieldMatchCacheMutex );

    QHash<QString, QList<Types::Property> >::ConstIterator it = d->fieldMatchCache.constFind( fieldName );
    if( it != d->fieldMatchCache.constEnd() ) {
        return it.value();
    }
    else {
        lock.unlock();

        QList<Nepomuk::Types::Property> results;

        //
        // Due to the limited number of properties in the database a REGEX filter
        // is actually faster than a fulltext query via bif:contains (this is what
        // experiments showed).
        //
        QString query = QString( "select distinct ?p where { "
                                 "graph ?g { "
                                 "?p a %1 . "
                                 "?p %2 ?l . "
                                 "FILTER(REGEX(STR(?l),'%3*','i') || REGEX(STR(?p),'%3*','i')) . "
                                 "} "
                                 "}" )
                        .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDF::Property() ) )
                        .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::label() ) )
                        .arg( fieldName );
        kDebug() << "Match query:" << query;

        Soprano::QueryResultIterator labelHits
            = Nepomuk::ResourceManager::instance()->mainModel()->executeQuery( query, Soprano::Query::QueryLanguageSparql );

        while ( labelHits.next() ) {
            QUrl property = labelHits.binding( "p" ).uri();
            results << property;
            kDebug() << "Found property match" << property;
        }

        lock.relock();
        d->fieldMatchCache.insert( fieldName, results );
        return results;
    }
}


Nepomuk::Query::Query Nepomuk::Query::QueryParser::parse( const QString& query ) const
{
    return parse( query, NoParserFlags );
}


Nepomuk::Query::Query Nepomuk::Query::QueryParser::parse( const QString& query, ParserFlags flags ) const
{
    // TODO: a "real" parser which can handle all of the Xesam user language
    //       This one for example does not handle nesting at all.

    Nepomuk::Query::Query final;
    QList<Term> terms;

    bool inOrBlock = false;
    bool inAndBlock = false;

    int pos = 0;

    // create local copies of the regexps for thread safety purposes
    const QRegExp resourceRx = s_regExpPool->resourceRx;
    const QRegExp propertyRx = s_regExpPool->propertyRx;
    const QRegExp fieldFieldRx = s_regExpPool->fieldFieldRx;
    const QRegExp fieldRx = s_regExpPool->fieldRx;
    const QRegExp plainTermRx = s_regExpPool->plainTermRx;

    while ( pos < query.length() ) {
        // skip whitespace
        while ( pos < query.length() && query[pos].isSpace() ) {
            kDebug() << "Skipping space at" << pos;
            ++pos;
        }

        Term term;

        if ( pos < query.length() ) {
            if ( resourceRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched resource term at" << pos << resourceRx.cap( 0 );
                term = ComparisonTerm( tryToBeIntelligentAboutParsingUrl( resourceRx.cap( 2 ) ),
                                       ResourceTerm( tryToBeIntelligentAboutParsingUrl( resourceRx.cap( 3 ) ) ),
                                       ComparisonTerm::Equal );
                if ( !positiveTerm( resourceRx.cap( 1 ) ) ) {
                    term = NegationTerm::negateTerm( term );
                }
                pos += resourceRx.matchedLength();
            }

            else if ( propertyRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched property term at" << pos << propertyRx.cap( 0 );
                ComparisonTerm ct;
                ct.setProperty( tryToBeIntelligentAboutParsingUrl( propertyRx.cap( 2 ) ) );
                ct.setSubTerm( LiteralTerm( createLiteral( propertyRx.cap( 4 ), flags&QueryTermGlobbing ) ) );
                QString comparator = propertyRx.cap( 3 );
                ct.setComparator( fieldTypeRelationFromString( comparator ) );
                pos += propertyRx.matchedLength();

                if ( !positiveTerm(propertyRx.cap( 1 ) ) ) {
                    term = NegationTerm::negateTerm( ct );
                }
                else {
                    term = ct;
                }
            }

            else if ( fieldFieldRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched field field term at" << pos
                         << fieldFieldRx.cap( 0 )
                         << fieldFieldRx.cap( 2 )
                         << fieldFieldRx.cap( 4 )
                         << fieldFieldRx.cap( 5 )
                         << fieldFieldRx.cap( 7 )
                         << fieldFieldRx.cap( 8 );
                ComparisonTerm ct;
                ct.setProperty( QUrl(stripQuotes( fieldFieldRx.cap( 2 ) )) );
                QString comparator = fieldFieldRx.cap( 4 );
                ct.setComparator( fieldTypeRelationFromString( comparator ) );
                ct.setSubTerm( ComparisonTerm( QUrl(stripQuotes( fieldFieldRx.cap( 5 ) )),
                                               LiteralTerm( createLiteral( fieldFieldRx.cap( 8 ), flags&QueryTermGlobbing ) ),
                                               fieldTypeRelationFromString( fieldFieldRx.cap( 7 ) ) ) );
                pos += fieldFieldRx.matchedLength();

                if ( !positiveTerm( fieldFieldRx.cap( 1 ) ) ) {
                    term = NegationTerm::negateTerm( ct );
                }
                else {
                    term = ct;
                }
            }

            else if ( fieldRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched field term at" << pos << fieldRx.cap( 0 ) << fieldRx.cap( 2 ) << fieldRx.cap( 4 ) << fieldRx.cap( 5 );
                if( stripQuotes ( fieldRx.cap( 2 ) ).compare( QString( "inFolder" ), Qt::CaseInsensitive ) == 0 ) {
                    KUrl url( fieldRx.cap( 5 ) );
                    kDebug() << "found include path" << url;
                    FileQuery fileQuery(final);
                    if ( positiveTerm( fieldRx.cap( 1 ) ) )
                        fileQuery.addIncludeFolder(url);
                    else
                        fileQuery.addExcludeFolder(url);
                    final = fileQuery;
                    pos += fieldRx.matchedLength();
                }
                else {
                    ComparisonTerm ct;
                    ct.setProperty( QUrl( stripQuotes( fieldRx.cap( 2 ) ) ) );
                    ct.setSubTerm( LiteralTerm( createLiteral( fieldRx.cap( 5 ), flags&QueryTermGlobbing ) ) );
                    QString comparator = fieldRx.cap( 4 );
                    ct.setComparator( fieldTypeRelationFromString( comparator ) );
                    pos += fieldRx.matchedLength();
                    if ( !positiveTerm(fieldRx.cap( 1 ) ) ) {
                        NegationTerm nt;
                        nt.setSubTerm( ct );
                        term = nt;
                    }
                    else {
                        term = ct;
                    }
                }
            }

            else if ( plainTermRx.indexIn( query, pos ) == pos ) {
                QString value = plainTermRx.cap( 2 );
                if ( d->orKeywords.contains( value.toLower() ) ) {
                    inOrBlock = true;
                }
                else if ( d->andKeywords.contains( value.toLower() ) ) {
                    inAndBlock = true;
                }
                else {
                    kDebug() << "matched literal at" << pos << value;
                    if( flags&DetectFilenamePattern && isFilenamePattern(value) ) {
                        term = createFilenamePatternTerm( value );
                    }
                    else {
                        term = LiteralTerm( createLiteral( value, flags&QueryTermGlobbing ) );
                    }
                    if ( !positiveTerm(plainTermRx.cap( 1 ) ) ) {
                        term = NegationTerm::negateTerm( term );
                    }
                }
                pos += plainTermRx.matchedLength();
            }

            else {
                kDebug() << "Invalid query at" << pos << query;
                return Query();
            }

            if ( term.isValid() ) {
                if ( inOrBlock && !terms.isEmpty() ) {
                    OrTerm orTerm;
                    orTerm.addSubTerm( terms.takeLast() );
                    orTerm.addSubTerm( term );
                    terms.append( orTerm );
                }
                else if ( inAndBlock && !terms.isEmpty() ) {
                    AndTerm andTerm;
                    andTerm.addSubTerm( terms.takeLast() );
                    andTerm.addSubTerm( term );
                    terms.append( andTerm );
                }
                else {
                    terms.append( term );
                }
            }
        }
    }

    if ( terms.count() == 1 ) {
        final.setTerm( terms[0] );
    }
    else if ( terms.count() > 0 ) {
        AndTerm t;
        t.setSubTerms( terms );
        final.setTerm( t );
    }

    d->m_invalidQuery = false;
    final.setTerm( mergeLiteralTerms( d->resolveFields( final.term() ) ) );
    if(d->m_invalidQuery) {
        return Query();
    }
    else {
        return final;
    }
}


// static
Nepomuk::Query::Query Nepomuk::Query::QueryParser::parseQuery( const QString& query )
{
    QueryParser parser;
    return parser.parse( query );
}


// static
Nepomuk::Query::Query Nepomuk::Query::QueryParser::parseQuery( const QString& query, ParserFlags flags )
{
    QueryParser parser;
    return parser.parse( query, flags );
}
