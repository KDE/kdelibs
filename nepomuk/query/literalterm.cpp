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
QString Nepomuk::Query::LiteralTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, QueryBuilderData* qbd ) const
{
    const QString v1 = qbd->uniqueVarName();
    const QString v2 = qbd->uniqueVarName();
    const QString v3 = qbd->uniqueVarName();
    const QString v4 = qbd->uniqueVarName();
    const QString containsPattern = createContainsPattern( v2, m_value.toString(), qbd );

    return QString::fromLatin1( "{ %1 %2 %3 . %4 } "
                                "UNION "
                                "{ %1 %2 %5 . %5 %6 %3 . %6 %7 %8 . %4 } . " )
        .arg( resourceVarName,
              v1,
              v2,
              containsPattern,
              v3,
              v4,
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subPropertyOf()),
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
    // each token with a negation flag
    QList<QPair<QString, bool> > containsTokens;
    QList<QPair<QString, bool> > regexTokens;

    // we only support AND xor OR, not both at the same time
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

        if( i == text.count()-1 ) {
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
                Q_FOREACH( const QString& subToken, subTokens ) {
                    const int i = subToken.indexOf( QRegExp(QLatin1String("[\\?\\*]")) );
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

    // convert the tokens into SPARQL filters
    QStringList filters;
    QStringList containsFilterTokens;
    for( int i = 0; i < containsTokens.count(); ++i ) {
        QString containsFilterToken;
        if( containsTokens[i].second )
            containsFilterToken += QLatin1String("NOT ");
        containsFilterToken += QString::fromLatin1("'%1'").arg(containsTokens[i].first);
        containsFilterTokens << containsFilterToken;
    }
    if( !containsFilterTokens.isEmpty() ) {
        filters << QString::fromLatin1("bif:contains(%1, \"%2\")")
                   .arg( varName,
                         containsFilterTokens.join( isUnion ? QLatin1String(" OR ") : QLatin1String(" AND ")) );
    }
    QStringList regexFilters;
    for( int i = 0; i < regexTokens.count(); ++i ) {
        QString regexFilter;
        if( regexTokens[i].second )
            regexFilter += QLatin1Char('!');
        regexFilter += QString::fromLatin1( "REGEX(%1, \"%2\")" )
                .arg( varName,
                      prepareRegexText(regexTokens[i].first) );
        filters << regexFilter;
    }

    return QString( QLatin1String("FILTER(") + filters.join( isUnion ? QLatin1String(" || ") : QLatin1String(" && ") ) + QLatin1String(") . ") );
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
