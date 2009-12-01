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

#include "comparisonterm.h"
#include "comparisonterm_p.h"
#include "querybuilderdata_p.h"
#include "literalterm.h"
#include "resourceterm.h"
#include "resource.h"

#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/Vocabulary/RDFS>

#include "literal.h"
#include "class.h"

#include <kdebug.h>


namespace {
    QString comparatorToString( Nepomuk::Query::ComparisonTerm::Comparator c )
    {
        switch( c ) {
        case Nepomuk::Query::ComparisonTerm::Greater:
            return QChar( '>' );
        case Nepomuk::Query::ComparisonTerm::Smaller:
            return QChar( '<' );
        case Nepomuk::Query::ComparisonTerm::GreaterOrEqual:
            return QLatin1String( ">=" );
        case Nepomuk::Query::ComparisonTerm::SmallerOrEqual:
            return QLatin1String( "<=" );
        default:
            return QString();
        }
    }
}


QString Nepomuk::Query::ComparisonTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, QueryBuilderData* qbd ) const
{
    //
    // 1. property range: literal
    // 1.1. operator =
    //      use a single pattern like: ?r <prop> "value"
    // 1.2. operator :
    //      use two patterns: ?r <prop> ?v . ?v bif:contains "'value*'"
    // 1.3. operator <,>,<=,>=
    //      use two patterns: ?r <prop> ?v . FILTER(?v < value)
    // fail if subterm is not a literal term
    //
    // 2. property range is class
    // 2.1. operator =
    // 2.1.1. literal subterm
    //        use two patterns like: ?r <prop> ?v . ?v rdfs:label "value"
    // 2.1.2. resource subterm
    //        use one pattern: ?r <prop> <res>
    // 2.1.3. subterm type and, or, comparision
    //        use one pattern and the subpattern: ?r <prop> ?v . subpattern(?v)
    // 2.2. operator :
    // 2.2.1. literal subterm
    //        use 3 pattern: ?r <prop> ?v . ?v rdfs:label ?l . ?l bif:contains "'value*'"
    // 2.2.2. resource subterm
    //        same as =
    // 2.2.3. subterm type and, or, comparision
    //        same as =
    // 2.3. operator <,>,<=,>=
    //      fail!
    //

    Nepomuk::Types::Property p( m_property );
    if ( p.literalRangeType().isValid() ) {
        if( !m_subTerm.isLiteralTerm() )
            kDebug() << "Incompatible subterm type:" << m_subTerm.type();
        if ( m_comparator == ComparisonTerm::Equal ||
             !m_subTerm.toLiteralTerm().value().isString() ) {
            return QString( "%1 %2 %3 . " )
                .arg( resourceVarName )
                .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                .arg( Soprano::Node::literalToN3( m_subTerm.toLiteralTerm().value() ) );
        }
        else if ( m_comparator == ComparisonTerm::Contains ) {
            QString v = qbd->uniqueVarName();
            return QString( "%1 %2 %3 . %3 bif:contains \"'%4*'\" . " )
                .arg( resourceVarName )
                .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                .arg( v )
                .arg( m_subTerm.toLiteralTerm().value().toString() );
        }
        else if ( m_comparator == ComparisonTerm::Regexp ) {
            QString v = qbd->uniqueVarName();
            return QString( "%1 %2 %3 . FILTER(REGEX(STR(%3), '%4', 'i')) . " )
                .arg( resourceVarName )
                .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                .arg( v )
                .arg( m_subTerm.toLiteralTerm().value().toString() );
        }
        else {
            QString v = qbd->uniqueVarName();
            return QString( "%1 %2 %3 . FILTER(%3%4\"%5\") . " )
                .arg( resourceVarName )
                .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                .arg( comparatorToString( m_comparator ) )
                .arg( v )
                .arg( m_subTerm.toLiteralTerm().value().toString() );
        }
    }

    else { // resource range
        if( !(m_comparator == ComparisonTerm::Equal ||
              m_comparator == ComparisonTerm::Contains ||
              m_comparator == ComparisonTerm::Regexp ))
            kDebug() << "Incompatible property range:" << p.range().uri();
        if ( m_subTerm.isLiteralTerm() ) {
            if ( m_comparator == ComparisonTerm::Equal ) {
                QString v1 = qbd->uniqueVarName();
                QString v2 = qbd->uniqueVarName();
                return QString( "%1 %2 %3 . %3 %4 %5 . %4 %6 %7 . " )
                    .arg( resourceVarName )
                    .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                    .arg( v1 )
                    .arg( v2 )
                    .arg( Soprano::Node::literalToN3( m_subTerm.toLiteralTerm().value() ) )
                    .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::subPropertyOf() ) ) // using crappy inferencing for now
                    .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::label() ) );
            }
            else if ( m_comparator == ComparisonTerm::Contains ) {
                QString v1 = qbd->uniqueVarName();
                QString v2 = qbd->uniqueVarName();
                QString v3 = qbd->uniqueVarName();
                return QString( "%1 %2 %3 . %3 %4 %5 . %4 %6 %7 . %5 bif:contains \"'%8*'\" . " )
                    .arg( resourceVarName )
                    .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                    .arg( v1 )
                    .arg( v2 )
                    .arg( v3 )
                    .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::subPropertyOf() ) ) // using crappy inferencing for now
                    .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::label() ) )
                    .arg( m_subTerm.toLiteralTerm().value().toString() );
            }
            else if ( m_comparator == ComparisonTerm::Regexp ) {
                QString v1 = qbd->uniqueVarName();
                QString v2 = qbd->uniqueVarName();
                QString v3 = qbd->uniqueVarName();
                return QString( "%1 %2 %3 . %3 %4 %5 . %4 %6 %7 . FILTER(REGEX(STR(%5)), '%8*', 'i') . " )
                    .arg( resourceVarName )
                    .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                    .arg( v1 )
                    .arg( v2 )
                    .arg( v3 )
                    .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::subPropertyOf() ) ) // using crappy inferencing for now
                    .arg( Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::label() ) )
                    .arg( m_subTerm.toLiteralTerm().value().toString() );
            }
            else {
                kDebug() << QString( "Invalid Term: comparator %1 cannot be used for matching to a resource!" ).arg( comparatorToString( m_comparator ) );
                return QString();
            }
        }
        else if ( m_subTerm.isResourceTerm() ) {
            return QString( "%1 %2 %3 . " )
                .arg( resourceVarName )
                .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                .arg( m_subTerm.d_ptr->toSparqlGraphPattern( resourceVarName, qbd ) );
        }
        else {
            QString v = qbd->uniqueVarName();
            return QString( "%1 %2 %3 . %4" )
                .arg( resourceVarName )
                .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
                .arg( v )
                .arg( m_subTerm.d_ptr->toSparqlGraphPattern( v, qbd ) );
        }
    }
}



bool Nepomuk::Query::ComparisonTermPrivate::equals( const TermPrivate* other ) const
{
    if ( other->m_type == m_type ) {
        const ComparisonTermPrivate* ctp = static_cast<const ComparisonTermPrivate*>( other );
        return( ctp->m_property == m_property &&
                ctp->m_comparator == m_comparator &&
                ctp->m_subTerm == m_subTerm );
    }
    else {
        return false;
    }
}


bool Nepomuk::Query::ComparisonTermPrivate::isValid() const
{
    return( SimpleTermPrivate::isValid() && m_property.isValid() );
}


QString Nepomuk::Query::ComparisonTermPrivate::toString() const
{
    return QString( "[%1 %2 %3]" )
        .arg( Soprano::Node::resourceToN3( m_property.uri() ) )
        .arg( comparatorToString( m_comparator ) )
        .arg( m_subTerm.d_ptr->toString() );
}


Nepomuk::Query::ComparisonTerm::ComparisonTerm()
    : SimpleTerm( new ComparisonTermPrivate() )
{
}


Nepomuk::Query::ComparisonTerm::ComparisonTerm( const ComparisonTerm& term )
    : SimpleTerm( term )
{
}


Nepomuk::Query::ComparisonTerm::ComparisonTerm( const Types::Property& property, const Term& term, Comparator comparator )
    : SimpleTerm( new ComparisonTermPrivate() )
{
    N_D( ComparisonTerm );
    d->m_property = property;
    d->m_subTerm = term;
    d->m_comparator = comparator;
}


Nepomuk::Query::ComparisonTerm::~ComparisonTerm()
{
}


Nepomuk::Query::ComparisonTerm& Nepomuk::Query::ComparisonTerm::operator=( const ComparisonTerm& term )
{
    d_ptr = term.d_ptr;
    return *this;
}


Nepomuk::Query::ComparisonTerm::Comparator Nepomuk::Query::ComparisonTerm::comparator() const
{
    N_D_CONST( ComparisonTerm );
    return d->m_comparator;
}


Nepomuk::Types::Property Nepomuk::Query::ComparisonTerm::property() const
{
    N_D_CONST( ComparisonTerm );
    return d->m_property;
}


void Nepomuk::Query::ComparisonTerm::setComparator( Comparator comparator )
{
    N_D( ComparisonTerm );
    d->m_comparator = comparator;
}


void Nepomuk::Query::ComparisonTerm::setProperty( const Types::Property& property )
{
    N_D( ComparisonTerm );
    d->m_property = property;
}
