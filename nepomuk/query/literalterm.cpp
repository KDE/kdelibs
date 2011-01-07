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
QString Nepomuk::Query::LiteralTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, const TermPrivate* parentTerm, QueryBuilderData* qbd ) const
{
    Q_UNUSED(parentTerm);

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
QString prepareQueryText( const QString& text )
{
    //
    // we try to be a little smart about creating the query text
    // by following a few simple rules:
    //
    // 1. enclose everything in quotes to be safe
    // 2. quotes in search terms are not handled. replace them with spaces
    // 3. replace double quotes with single quotes
    // [4. wildcards can only be used if they are preceeded by at least 4 chars]
    //

    QString s = text.simplified();
    if( s.isEmpty() )
        return s;

    // strip quotes
    if( s[0] == '"' || s[0] == '\'' ) {
        s = s.mid(1);
    }
    if( !s.isEmpty() &&
        ( s[s.length()-1] == '"' || s[s.length()-1] == '\'' ) ) {
        s.truncate(s.length()-1);
    }

    // replace quotes with spaces
    s.replace( '"', ' ' );
    s.replace( '\'', ' ' );

    // add quotes
    s = '\'' + s + '\'';

    return s;
}

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
    const int i = text.indexOf( QRegExp(QLatin1String("[\\?\\*]")) );

    //
    // Virtuoso needs four leading chars when using wildcards. Thus, if there is less (this includes 0) we fall back to the slower regex filter
    //
    if( i < 0 || i > 3 ) {
        const QString finalText = prepareQueryText( text );

        QString scoringPattern;
        if( qbd->query()->m_fullTextScoringEnabled ) {
            scoringPattern = QString::fromLatin1("OPTION (score %1) ").arg(qbd->createScoringVariable());
        }
        qbd->addFullTextSearchTerm( varName, finalText );

        return QString::fromLatin1( "%1 bif:contains \"%2\" %3. " )
                .arg( varName,
                     finalText,
                     scoringPattern );
    }
    else {
        return QString::fromLatin1( "FILTER(REGEX(%1, \"%2\")) . " )
                .arg( varName, prepareRegexText(text) );
    }
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
