/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>

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

#include "term.h"
#include "term_p.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "andterm.h"
#include "orterm.h"
#include "negationterm.h"
#include "comparisonterm.h"
#include "literalterm_p.h"
#include "resourceterm_p.h"
#include "andterm_p.h"
#include "orterm_p.h"
#include "negationterm_p.h"
#include "comparisonterm_p.h"

#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QDebug>

#include "property.h"


Nepomuk::Query::Term::Term()
    : d_ptr( new TermPrivate() )
{
}


Nepomuk::Query::Term::Term( const Term& other )
    : d_ptr( other.d_ptr )
{
}


Nepomuk::Query::Term::Term( TermPrivate* d )
    : d_ptr( d )
{
}


Nepomuk::Query::Term::~Term()
{
}


Nepomuk::Query::Term& Nepomuk::Query::Term::operator=( const Term& other )
{
    d_ptr = other.d_ptr;
    return *this;
}


bool Nepomuk::Query::Term::isValid() const
{
    return d_ptr->isValid();
}


Nepomuk::Query::Term::Type Nepomuk::Query::Term::type() const
{
    return d_ptr->m_type;
}


bool Nepomuk::Query::Term::isLiteralTerm() const
{
    return type() == Literal;
}


bool Nepomuk::Query::Term::isResourceTerm() const
{
    return type() == Resource;
}


bool Nepomuk::Query::Term::isNegationTerm() const
{
    return type() == Negation;
}


bool Nepomuk::Query::Term::isAndTerm() const
{
    return type() == And;
}


bool Nepomuk::Query::Term::isOrTerm() const
{
    return type() == Or;
}


bool Nepomuk::Query::Term::isComparisonTerm() const
{
    return type() == Comparison;
}


Nepomuk::Query::LiteralTerm Nepomuk::Query::Term::toLiteralTerm() const
{
    if ( isLiteralTerm() ) {
        return *static_cast<const LiteralTerm*>( this );
    }
    else
        return LiteralTerm();
}


Nepomuk::Query::ResourceTerm Nepomuk::Query::Term::toResourceTerm() const
{
    if ( isResourceTerm() )
        return *static_cast<const ResourceTerm*>( this );
    else
        return ResourceTerm();
}


Nepomuk::Query::NegationTerm Nepomuk::Query::Term::toNegationTerm() const
{
    if ( isNegationTerm() )
        return *static_cast<const NegationTerm*>( this );
    else
        return NegationTerm();
}


Nepomuk::Query::AndTerm Nepomuk::Query::Term::toAndTerm() const
{
    if ( isAndTerm() )
        return *static_cast<const AndTerm*>( this );
    else
        return AndTerm();
}


Nepomuk::Query::OrTerm Nepomuk::Query::Term::toOrTerm() const
{
    if ( isOrTerm() )
        return *static_cast<const OrTerm*>( this );
    else
        return OrTerm();
}


Nepomuk::Query::ComparisonTerm Nepomuk::Query::Term::toComparisonTerm() const
{
    if ( isComparisonTerm() )
        return *static_cast<const ComparisonTerm*>( this );
    else
        return ComparisonTerm();
}


#define CONVERT_AND_RETURN( Class ) \
    if ( !is##Class() )                                       \
        d_ptr = new Class##Private();                         \
    return *static_cast<Class*>( this )

Nepomuk::Query::LiteralTerm& Nepomuk::Query::Term::toLiteralTerm()
{
    CONVERT_AND_RETURN( LiteralTerm );
}


Nepomuk::Query::ResourceTerm& Nepomuk::Query::Term::toResourceTerm()
{
    CONVERT_AND_RETURN( ResourceTerm );
}


Nepomuk::Query::NegationTerm& Nepomuk::Query::Term::toNegationTerm()
{
    CONVERT_AND_RETURN( NegationTerm );
}


Nepomuk::Query::AndTerm& Nepomuk::Query::Term::toAndTerm()
{
    CONVERT_AND_RETURN( AndTerm );
}


Nepomuk::Query::OrTerm& Nepomuk::Query::Term::toOrTerm()
{
    CONVERT_AND_RETURN( OrTerm );
}


Nepomuk::Query::ComparisonTerm& Nepomuk::Query::Term::toComparisonTerm()
{
    CONVERT_AND_RETURN( ComparisonTerm );
}


bool Nepomuk::Query::Term::operator==( const Term& other ) const
{
    return d_ptr->equals( other.d_ptr );
}


QDebug Nepomuk::Query::Term::operator<<( QDebug dbg ) const
{
    return dbg << d_ptr->toString();
}


QDebug operator<<( QDebug dbg, const Nepomuk::Query::Term& term )
{
    return term.operator<<( dbg );
}


uint Nepomuk::Query::qHash( const Nepomuk::Query::Term& term )
{
    switch( term.type() ) {
    case Nepomuk::Query::Term::Literal:
        return( qHash( term.toLiteralTerm().value().toString() ) );

    case Nepomuk::Query::Term::Comparison:
        return( qHash( term.toComparisonTerm().property().uri().toString() )<<24 |
                qHash( term.toComparisonTerm().subTerm() )<<16 |
                ( uint )term.toComparisonTerm().comparator()<<8 );

    case Nepomuk::Query::Term::Negation:
        return 1;

    case Nepomuk::Query::Term::And:
    case Nepomuk::Query::Term::Or: {
        uint h = ( uint )term.type();
        QList<Nepomuk::Query::Term> subTerms = static_cast<const GroupTerm&>( term ).subTerms();
        for ( int i = 0; i < subTerms.count(); ++i ) {
            h |= ( qHash( subTerms[i] )<<i );
        }
        return h;
    }

    default:
        return 0;
    }
}


/// We need to overload QSharedDataPointer::clone to make sure the correct subclasses are created
/// when detaching. The default implementation would always call TermPrivate( const TermPrivate& )
template<> Nepomuk::Query::TermPrivate* QSharedDataPointer<Nepomuk::Query::TermPrivate>::clone()
{
    return d->clone();
}
