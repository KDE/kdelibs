/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2009 Sebastian Trueg <trueg@kde.org>

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
    QString v1 = qbd->uniqueVarName();
    QString v2 = qbd->uniqueVarName();
    QString v3 = qbd->uniqueVarName();
    QString v4 = qbd->uniqueVarName();
    // { ?r ?v1 ?v2 . ?v2 bif:contains XXX . } UNION { ?r ?v1 ?v3 . ?v3 ?v4 ?v2 . ?v4 rdfs:subPropertyOf rdfs:label . ?v2 bif:contains XXX . } .
    return QString::fromLatin1( "{ %1 %2 %3 . %3 bif:contains \"'%4'\" . } "
                                "UNION "
                                "{ %1 %2 %5 . %5 %6 %3 . %6 %7 %8 . %3 bif:contains \"'%4'\" . } . " )
        .arg( resourceVarName,
              v1,
              v2,
              queryText(),
              v3,
              v4,
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::subPropertyOf()),
              Soprano::Node::resourceToN3(Soprano::Vocabulary::RDFS::label()) );
}


QString Nepomuk::Query::LiteralTermPrivate::toString() const
{
    return QString( "[Literal %1]" ).arg( Soprano::Node::literalToN3( m_value ) );
}


QString Nepomuk::Query::LiteralTermPrivate::queryText() const
{
    // Virtuoso 6 has a min of 4 leading chars before a wildcard.
    // Thus, we do not use one with less chars.

    QString s = m_value.toString();
     if( s.length() > 3 &&
         !s.endsWith(QLatin1String("*")) &&
         !s.endsWith(QLatin1String("?")) ) {
        s += QLatin1String("*");
    }
    return s;
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
