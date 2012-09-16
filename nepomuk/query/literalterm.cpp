/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009-2012 Sebastian Trueg <trueg@kde.org>

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

#include "literalterm.h"
#include "literalterm_p.h"
#include "querybuilderdata_p.h"
#include "query_p.h"

#include <Soprano/Node>
#include <Soprano/Vocabulary/RDFS>

Nepomuk::Query::LiteralTermPrivate::LiteralTermPrivate()
{
    m_type = Term::Literal;
}


bool Nepomuk::Query::LiteralTermPrivate::equals( const TermPrivate* other ) const
{
    if ( other->m_type == m_type ) {
        const LiteralTermPrivate* rtp = static_cast<const LiteralTermPrivate*>( other );
        return rtp->m_value == m_value;
    }
    else {
        return false;
    }
}


//
// A LiteralTerm not used in a ComparisonTerm is a "classical" plain text search term. That would mean "?r ?p ?v . ?v bif:contains 'foobar' . "
// But since many relations like nao:hasTag or nmm:performer or similar are considered as plain text fields we extend the pattern by adding
// relations to resources that have labels containing the query text.
//
QString Nepomuk::Query::LiteralTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, const TermPrivate* parentTerm, const QString &additionalFilters, QueryBuilderData *qbd ) const
{
    Q_UNUSED(parentTerm);

    if( m_value.toString().isEmpty() )
        return QString();

    const QString p1 = qbd->uniqueVarName();
    const QString v1 = qbd->uniqueVarName();
    const QString r2 = qbd->uniqueVarName();
    const QString containsPattern = createContainsPattern( v1, m_value.toString(), qbd );

    // { ?r ?p1 ?v1 . containsPattern(v1) }
    // UNION
    // { ?r ?p1 ?r2 . ?r2 rdfs:label ?v1 . containsPattern(v1) } .
    return QString::fromLatin1( "{ %1 %2 %3 . %4 } "
                                "UNION "
                                "{ %1 %2 %5 . %5 %6 %3 . %4 } . " )
        .arg( resourceVarName,
              p1,
              v1,
              containsPattern + additionalFilters,
              r2,
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::label()) );
}


namespace {
QString prepareRegexText( const QString& text )
{
    QString filterRxStr = QRegExp::escape( text );
    filterRxStr.replace( "\\*", QLatin1String( ".*" ) );
    filterRxStr.replace( "\\?", QLatin1String( "." ) );
    filterRxStr.replace( '\\',"\\\\" );
    return filterRxStr;
}
}


QString Nepomuk::Query::LiteralTermPrivate::createContainsPattern( const QString& varName, const QString& text, Nepomuk::Query::QueryBuilderData* qbd )
{
    if( text.isEmpty() )
        return QString();

    // each token with a negation flag
    QList<QPair<QString, bool> > containsTokens;
    QList<QPair<QString, bool> > regexTokens;

    // we only support AND or OR, not both at the same time
    // TODO: Fix this. Virtuoso supports a combination of both
    bool isUnion = false;

    // gather all the tokens
    bool inQuotes = false;
    QString currentToken;
    bool nextIsNegated = false;
    int i = 0;
    while( i < text.length() ) {
        const QChar& c = text[i];
        bool tokenEnd = false;

        if( c == QChar('"') || c == QChar('\'') ) {
            inQuotes = !inQuotes;
            tokenEnd = !inQuotes;
        }
        else if( c.isSpace() && !inQuotes ) {
            tokenEnd = true;
        }
        else {
            currentToken.append(c);
        }

        if( i == text.length()-1 ) {
            tokenEnd = true;
        }

        if( tokenEnd && !currentToken.isEmpty() ) {
            //
            // Handle the three special tokens supported in Virtuoso's full text search engine we support (there is also "near" which we do not handle yet)
            //
            if( currentToken.toLower() == QLatin1String("and") ) {
                isUnion = false;
            }
            else if( currentToken.toLower() == QLatin1String("or") ) {
                isUnion = true;
            }
            else if( currentToken.toLower() == QLatin1String("not") ) {
                nextIsNegated = true;
            }
            else {
                QPair<QString, bool> currentTokenPair = qMakePair( currentToken, nextIsNegated );

                //
                // Virtuoso needs four leading chars when using wildcards. Thus, if there is less (this includes 0) we fall back to the slower regex filter
                //
                const QStringList subTokens = currentToken.split( QLatin1Char(' '), QString::SkipEmptyParts );
                bool needsRegex = false;
                QRegExp regex(QLatin1String("[\\?\\*]")); // The regex used to check if we needs a regex
                Q_FOREACH( const QString& subToken, subTokens ) {
                    const int i = subToken.indexOf( regex );
                    if( i >= 0 && i < 4 ) {
                        needsRegex = true;
                        break;
                    }
                }
                if( !needsRegex ) {
                    containsTokens << currentTokenPair;
                }
                else {
                    regexTokens << currentTokenPair;
                }
            }

            nextIsNegated = false;
            currentToken.clear();
        }

        ++i;
    }

    // add optional NOT terms to the contains tokens and build the search excerpts terms
    QStringList containsFilterTokens;
    QStringList fullTextTerms;
    for( int i = 0; i < containsTokens.count(); ++i ) {
        QString containsFilterToken;
        if( containsTokens[i].second )
            containsFilterToken += QLatin1String("NOT ");
        containsFilterToken += QString::fromLatin1("'%1'").arg(containsTokens[i].first);
        containsFilterTokens << containsFilterToken;

        // we only want to show excerpt with the actually searched terms
        if( !containsTokens[i].second )
            fullTextTerms << containsTokens[i].first;
    }
    if( !fullTextTerms.isEmpty() ) {
        qbd->addFullTextSearchTerms( varName, fullTextTerms );
    }

    const QString finalContainsToken = containsFilterTokens.join( isUnion ? QLatin1String(" OR ") : QLatin1String(" AND "));

    // convert the regex tokens into SPARQL filters
    QStringList filters;
    for( int i = 0; i < regexTokens.count(); ++i ) {
        QString regexFilter;
        if( regexTokens[i].second )
            regexFilter += QLatin1Char('!');
        regexFilter += QString::fromLatin1( "REGEX(%1, \"%2\", 'i')" )
                .arg( varName,
                      prepareRegexText(regexTokens[i].first) );
        filters << regexFilter;
    }

    //
    // with the current filter design we can only support full-text scoring if we either
    // only have contains pattern or if isUnion is false. In the latter case we can simply
    // use a normal graph pattern and a FILTER.
    //
    QString containsPattern;
    if( !containsFilterTokens.isEmpty() &&
            qbd->query()->m_fullTextScoringEnabled &&
            (regexTokens.isEmpty() || !isUnion) ) {
        containsPattern = QString::fromLatin1("%1 bif:contains \"%2\" OPTION (score %3) . ")
                .arg( varName,
                      finalContainsToken,
                      qbd->createScoringVariable() );
    }

    //
    // The fallback is to add bif:contains as a filter. This syntax does not support scoring though.
    //
    else if( !containsFilterTokens.isEmpty() ) {
        filters << QString::fromLatin1("bif:contains(%1, \"%2\")").arg( varName, finalContainsToken );
    }

    QString filterPattern;
    if(!filters.isEmpty())
        filterPattern = QString( QLatin1String("FILTER(") + filters.join( isUnion ? QLatin1String(" || ") : QLatin1String(" && ") ) + QLatin1String(") . ") );

    return filterPattern + containsPattern;
}


Nepomuk::Query::LiteralTerm::LiteralTerm( const LiteralTerm& term )
    : Term( term )
{
}


Nepomuk::Query::LiteralTerm::LiteralTerm( const Soprano::LiteralValue& value )
    : Term( new LiteralTermPrivate() )
{
    setValue( value );
}


Nepomuk::Query::LiteralTerm::~LiteralTerm()
{
}


Nepomuk::Query::LiteralTerm& Nepomuk::Query::LiteralTerm::operator=( const LiteralTerm& term )
{
    d_ptr = term.d_ptr;
    return *this;
}


Soprano::LiteralValue Nepomuk::Query::LiteralTerm::value() const
{
    N_D_CONST( LiteralTerm );
    return d->m_value;
}


void Nepomuk::Query::LiteralTerm::setValue( const Soprano::LiteralValue& value )
{
    N_D( LiteralTerm );
    d->m_value = value;
}
