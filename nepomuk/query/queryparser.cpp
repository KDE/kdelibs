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

#include <QtCore/QRegExp>
#include <QtCore/QSet>

#include <kdebug.h>
#include <klocale.h>

#include "resourcemanager.h"
#include "property.h"

#include <Soprano/Node>
#include <Soprano/Model>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDFS>
#include <Soprano/Vocabulary/RDF>


using namespace Nepomuk::Query;

namespace {
    // a field differs from a plain term in that it does never allow comparators
    QString s_fieldNamePattern( "([^\\s\"':=<>]+|(?:([\"'])[^\"':=<>]+\\%1))" );
    QString s_plainTermPattern( "([^\\s\"':=<>]+|(?:([\"'])[^\"']+\\%1))" );
    QString s_inExclusionPattern( "([\\+\\-]?)" );
    QString s_uriPattern( "<([^<>]+)>" );
    QString s_comparatorPattern( "(:|\\<=|\\>=|=|\\<|\\>)" );

    // match a simple search text
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - the search text (including optional paranthesis)
    QRegExp s_plainTermRx( s_inExclusionPattern + s_plainTermPattern.arg( 3 ) );

    // match a field search term: fieldname + relation (:, =, etc) + search text with optional paranthesis
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - fieldname
    //           3 - relation
    //           4 - search text (including optional paranthesis)
    QRegExp s_fieldRx( s_inExclusionPattern + s_fieldNamePattern.arg( 3 ) + s_comparatorPattern + s_plainTermPattern.arg( 6 ) );

    // match a property URI search term: property URI + relation (:, =, etc) + search text with optional paranthesis
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - property URI
    //           3 - relation
    //           4 - search text (including optional paranthesis)
    QRegExp s_propertyRx( s_inExclusionPattern + s_uriPattern + s_comparatorPattern + s_plainTermPattern.arg( 5 ) );

    // match a property URI search term: property URI + relation (:, =, etc) + resource URI
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - property URI
    //           3 - resource URI
    QRegExp s_resourceRx( s_inExclusionPattern + s_uriPattern + "(?::|=)" + s_uriPattern );

    QRegExp s_fieldFieldRx( s_inExclusionPattern + s_fieldNamePattern.arg( 3 ) + s_comparatorPattern + "\\(" +  s_fieldNamePattern.arg( 6 ) + s_comparatorPattern + s_plainTermPattern.arg( 9 ) + "\\)" );

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
            return QUrl::fromEncoded( s.toAscii() );
        }
        else {
            return QUrl( s );
        }
    }

    Soprano::LiteralValue createLiteral( const QString& s_, bool globbing ) {
        bool hadQuotes = false;
        QString s = stripQuotes( s_, &hadQuotes );
        bool b = false;
        int i = s.toInt( &b );
        if ( b )
            return Soprano::LiteralValue( i );
        double d = s.toDouble( &b );
        if ( b )
            return Soprano::LiteralValue( d );

        //
        // we can only do query term globbing for strings longer than 3 chars
        //
        if( !hadQuotes && globbing && s.length() > 3 && !s.endsWith('*') && !s.endsWith('?') )
            return s + '*';
        else
            return s;
    }

    bool positiveTerm( const QString& s ) {
        if(s.isEmpty())
            return true;
        else if(s == "+")
            return true;
        else if(s == "-")
            return false;
        else //unrecognized capture
            return true;
    }

    // TODO do this dorectly while parsing (but only once we have a real parser based on bison or whatever
    Nepomuk::Query::Term resolveFields( const Nepomuk::Query::Term& term, const Nepomuk::Query::QueryParser* parser )
    {
        switch( term.type() ) {
        case Nepomuk::Query::Term::And:
        case Nepomuk::Query::Term::Or: {
            QList<Nepomuk::Query::Term> newSubTerms;
            foreach( const Nepomuk::Query::Term& t, static_cast<const Nepomuk::Query::GroupTerm&>( term ).subTerms() ) {
                Nepomuk::Query::Term resolvedTerm = resolveFields(t, parser);
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
            return Nepomuk::Query::NegationTerm::negateTerm( resolveFields( term.toNegationTerm().subTerm(), parser ) );
        }


        case Nepomuk::Query::Term::Comparison: {
            Nepomuk::Query::ComparisonTerm newTerm;
            newTerm.setComparator( term.toComparisonTerm().comparator() );
            newTerm.setProperty( term.toComparisonTerm().property() );
            newTerm.setSubTerm( resolveFields( term.toComparisonTerm().subTerm(), parser ) );

            // A very dumb test to see if the property is set or not: does the URI have a scheme.
            // With a proper parser and in-place property matching there will be no need for this anymore
            if ( newTerm.property().uri().scheme().isEmpty() ) {
                QList<Nepomuk::Types::Property> properties = parser->matchProperty( term.toComparisonTerm().property().uri().toString() );
                if ( properties.count() > 0 ) {
                    if ( properties.count() == 1 ) {
                        newTerm.setProperty( properties.first() );
                        return newTerm;
                    }
                    else {
                        Nepomuk::Query::OrTerm orTerm;
                        foreach( const Nepomuk::Types::Property& property, properties ) {
                            Nepomuk::Query::ComparisonTerm t( newTerm );
                            t.setProperty( property );
                            orTerm.addSubTerm( t );
                        }
                        return orTerm;
                    }
                }
                else {
                    return Nepomuk::Query::Term();
                }
            }
        }

        default:
            return term;
        }
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
}


Nepomuk::Query::Query Nepomuk::Query::QueryParser::parseQuery( const QString& query )
{
    QueryParser parser;
    return parser.parse( query );
}


class Nepomuk::Query::QueryParser::Private
{
public:
    QSet<QString> andKeywords;
    QSet<QString> orKeywords;
    mutable QHash<QString, QList<Types::Property> > fieldMatchCache;
};


Nepomuk::Query::QueryParser::QueryParser()
    : d( new Private() )
{
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

    QHash<QString, QList<Types::Property> >::ConstIterator it = d->fieldMatchCache.constFind( fieldName );
    if( it != d->fieldMatchCache.constEnd() ) {
        return it.value();
    }
    else {
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
    while ( pos < query.length() ) {
        // skip whitespace
        while ( pos < query.length() && query[pos].isSpace() ) {
            kDebug() << "Skipping space at" << pos;
            ++pos;
        }

        Term term;

        if ( pos < query.length() ) {
            if ( s_resourceRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched resource term at" << pos << s_resourceRx.cap( 0 );
                term = ComparisonTerm( tryToBeIntelligentAboutParsingUrl( s_resourceRx.cap( 2 ) ),
                                       ResourceTerm( tryToBeIntelligentAboutParsingUrl( s_resourceRx.cap( 3 ) ) ),
                                       ComparisonTerm::Equal );
                if ( !positiveTerm(s_resourceRx.cap( 1 ) ) ) {
                    term = NegationTerm::negateTerm( term );
                }
                pos += s_resourceRx.matchedLength();
            }

            else if ( s_propertyRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched property term at" << pos << s_propertyRx.cap( 0 );
                ComparisonTerm ct;
                ct.setProperty( tryToBeIntelligentAboutParsingUrl( s_propertyRx.cap( 2 ) ) );
                ct.setSubTerm( LiteralTerm( createLiteral( s_propertyRx.cap( 4 ), flags&QueryTermGlobbing ) ) );
                QString comparator = s_propertyRx.cap( 3 );
                ct.setComparator( fieldTypeRelationFromString( comparator ) );
                pos += s_propertyRx.matchedLength();

                if ( !positiveTerm(s_propertyRx.cap( 1 ) ) ) {
                    term = NegationTerm::negateTerm( ct );
                }
                else {
                    term = ct;
                }
            }

            else if ( s_fieldFieldRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched field field term at" << pos
                         << s_fieldFieldRx.cap( 0 )
                         << s_fieldFieldRx.cap( 2 )
                         << s_fieldFieldRx.cap( 4 )
                         << s_fieldFieldRx.cap( 5 )
                         << s_fieldFieldRx.cap( 7 )
                         << s_fieldFieldRx.cap( 8 );
                ComparisonTerm ct;
                ct.setProperty( QUrl(stripQuotes( s_fieldFieldRx.cap( 2 ) )) );
                QString comparator = s_fieldFieldRx.cap( 4 );
                ct.setComparator( fieldTypeRelationFromString( comparator ) );
                ct.setSubTerm( ComparisonTerm( QUrl(stripQuotes( s_fieldFieldRx.cap( 5 ) )),
                                               LiteralTerm( createLiteral( s_fieldFieldRx.cap( 8 ), flags&QueryTermGlobbing ) ),
                                               fieldTypeRelationFromString( s_fieldFieldRx.cap( 7 ) ) ) );
                pos += s_fieldFieldRx.matchedLength();

                if ( !positiveTerm(s_fieldFieldRx.cap( 1 ) ) ) {
                    term = NegationTerm::negateTerm( ct );
                }
                else {
                    term = ct;
                }
            }

            else if ( s_fieldRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched field term at" << pos << s_fieldRx.cap( 0 ) << s_fieldRx.cap( 2 ) << s_fieldRx.cap( 4 ) << s_fieldRx.cap( 5 );
                if( stripQuotes ( s_fieldRx.cap( 2 ) ).compare( QString( "inFolder" ), Qt::CaseInsensitive ) == 0 ) {
                    KUrl url( s_fieldRx.cap( 5 ) );
                    kDebug() << "found include path" << url;
                    final.d->m_isFileQuery = true;
                    if ( positiveTerm( s_fieldRx.cap( 1 ) ) )
                        final.d->m_includeFolders << url;
                    else
                        final.d->m_excludeFolders << url;
                    pos += s_fieldRx.matchedLength();
                }
                else {
                    ComparisonTerm ct;
                    ct.setProperty( QUrl( stripQuotes( s_fieldRx.cap( 2 ) ) ) );
                    ct.setSubTerm( LiteralTerm( createLiteral( s_fieldRx.cap( 5 ), flags&QueryTermGlobbing ) ) );
                    QString comparator = s_fieldRx.cap( 4 );
                    ct.setComparator( fieldTypeRelationFromString( comparator ) );
                    pos += s_fieldRx.matchedLength();
                    if ( !positiveTerm(s_fieldRx.cap( 1 ) ) ) {
                        NegationTerm nt;
                        nt.setSubTerm( ct );
                        term = nt;
                    }
                    else {
                        term = ct;
                    }
                }
            }

            else if ( s_plainTermRx.indexIn( query, pos ) == pos ) {
                QString value = s_plainTermRx.cap( 2 );
                if ( d->orKeywords.contains( value.toLower() ) ) {
                    inOrBlock = true;
                }
                else if ( d->andKeywords.contains( value.toLower() ) ) {
                    inAndBlock = true;
                }
                else {
                    kDebug() << "matched literal at" << pos << value;
                    term = LiteralTerm( createLiteral( value, flags&QueryTermGlobbing ) );
                    if ( !positiveTerm(s_plainTermRx.cap( 1 ) ) ) {
                        term = NegationTerm::negateTerm( term );
                    }
                }
                pos += s_plainTermRx.matchedLength();
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

    final.setTerm( resolveFields( final.term(), this ) );
    return final;
}
