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

#include "comparisonterm.h"
#include "comparisonterm_p.h"
#include "querybuilderdata_p.h"
#include "literalterm.h"
#include "literalterm_p.h"
#include "resourceterm.h"
#include "resource.h"

#include <Soprano/LiteralValue>
#include <Soprano/Node>
#include <Soprano/Vocabulary/RDFS>

#include "literal.h"
#include "class.h"

#include <kdebug.h>


namespace {

    QString varInAggregateFunction( Nepomuk::Query::ComparisonTerm::AggregateFunction f, const QString& varName )
    {
        switch( f ) {
        case Nepomuk::Query::ComparisonTerm::Count:
            return QString::fromLatin1("count(%1)").arg(varName);
        case Nepomuk::Query::ComparisonTerm::DistinctCount:
            return QString::fromLatin1("count(distinct %1)").arg(varName);
        case Nepomuk::Query::ComparisonTerm::Max:
            return QString::fromLatin1("max(%1)").arg(varName);
        case Nepomuk::Query::ComparisonTerm::Min:
            return QString::fromLatin1("min(%1)").arg(varName);
        case Nepomuk::Query::ComparisonTerm::Sum:
            return QString::fromLatin1("sum(%1)").arg(varName);
        case Nepomuk::Query::ComparisonTerm::DistinctSum:
            return QString::fromLatin1("sum(distinct %1)").arg(varName);
        case Nepomuk::Query::ComparisonTerm::Average:
            return QString::fromLatin1("avg(%1)").arg(varName);
        case Nepomuk::Query::ComparisonTerm::DistinctAverage:
            return QString::fromLatin1("avg(distinct %1)").arg(varName);
        default:
            return QString();
        }
    }
}

QString Nepomuk::Query::comparatorToString( Nepomuk::Query::ComparisonTerm::Comparator c )
{
    switch( c ) {
    case Nepomuk::Query::ComparisonTerm::Contains:
        return QChar( ':' );
    case Nepomuk::Query::ComparisonTerm::Equal:
        return QChar( '=' );
    case Nepomuk::Query::ComparisonTerm::Regexp:
        return QLatin1String( "regex" );
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


Nepomuk::Query::ComparisonTerm::Comparator Nepomuk::Query::stringToComparator( const QStringRef& c )
{
    if( c == QChar( '=' ) )
        return Nepomuk::Query::ComparisonTerm::Equal;
    else if( c == QLatin1String( "regex" ) )
        return Nepomuk::Query::ComparisonTerm::Regexp;
    else if( c == QChar( '>' ) )
        return Nepomuk::Query::ComparisonTerm::Greater;
    else if( c == QChar( '<' ) )
        return Nepomuk::Query::ComparisonTerm::Smaller;
    else if( c == QLatin1String( ">=" ) )
        return Nepomuk::Query::ComparisonTerm::GreaterOrEqual;
    else if( c == QLatin1String( "<=" ) )
        return Nepomuk::Query::ComparisonTerm::SmallerOrEqual;
    else
        return Nepomuk::Query::ComparisonTerm::Contains;
}

QString Nepomuk::Query::ComparisonTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, QueryBuilderData* qbd ) const
{
    //
    // 1. property range: literal
    // 1.1. operator =
    //      use a single pattern like: ?r <prop> "value"
    // 1.2. operator :
    //      use two patterns: ?r <prop> ?v . ?v bif:contains "'value'"
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
    //        use 3 pattern: ?r <prop> ?v . ?v rdfs:label ?l . ?l bif:contains "'value'"
    // 2.2.2. resource subterm
    //        same as =
    // 2.2.3. subterm type and, or, comparision
    //        same as =
    // 2.3. operator <,>,<=,>=
    //      fail!
    //

    if ( !m_subTerm.isValid() ) {
        QString prop = propertyToString( qbd );
        QString ov = getMainVariableName( qbd );
        return QString::fromLatin1( "%1 %2 %3 . " )
            .arg( resourceVarName, prop, ov );
    }

    else if ( m_property.literalRangeType().isValid() ) {
        if( !m_subTerm.isLiteralTerm() )
            kDebug() << "Incompatible subterm type:" << m_subTerm.type();
        if ( m_comparator == ComparisonTerm::Equal ) {
            return QString::fromLatin1( "%1 %2 %3 . " )
                .arg( resourceVarName,
                      propertyToString( qbd ),
                      Soprano::Node::literalToN3( m_subTerm.toLiteralTerm().value() ) );
        }
        else if ( m_comparator == ComparisonTerm::Contains ) {
            QString v = getMainVariableName(qbd);
            QString scoringPattern;
            if( !(qbd->flags()&Query::WithoutScoring) ) {
                scoringPattern = QString::fromLatin1("OPTION (score %1) ").arg(qbd->createScoringVariable());
            }
            return QString::fromLatin1( "%1 %2 %3 . %3 bif:contains \"%4\" %5. " )
                .arg( resourceVarName,
                      propertyToString( qbd ),
                      v,
                      static_cast<const LiteralTermPrivate*>(m_subTerm.toLiteralTerm().d_ptr.constData())->queryText(),
                      scoringPattern );
        }
        else if ( m_comparator == ComparisonTerm::Regexp ) {
            QString v = getMainVariableName(qbd);
            return QString::fromLatin1( "%1 %2 %3 . FILTER(REGEX(STR(%3), '%4', 'i')) . " )
                .arg( resourceVarName,
                      propertyToString( qbd ),
                      v,
                      m_subTerm.toLiteralTerm().value().toString() );
        }
        else {
            QString v = getMainVariableName(qbd);
            return QString::fromLatin1( "%1 %2 %3 . FILTER(%3%4%5) . " )
                .arg( resourceVarName,
                      propertyToString( qbd ),
                      v,
                      comparatorToString( m_comparator ),
                      Soprano::Node::literalToN3(m_subTerm.toLiteralTerm().value()) );
        }
    }

    else { // resource range
        if( !(m_comparator == ComparisonTerm::Equal ||
              m_comparator == ComparisonTerm::Contains ||
              m_comparator == ComparisonTerm::Regexp ))
            kDebug() << "Incompatible property range:" << m_property.range().uri();

        //
        // The core pattern is always the same: we match to resources that have a certain
        // property defined. The value of that property is filled in below.
        //
        QString corePattern;
        QString subject;
        QString object;
        if( m_inverted && !m_subTerm.isLiteralTerm() ) {
            subject = QLatin1String("%1"); // funny way to have a resulting string which takes only one arg
            object = resourceVarName;
        }
        else {
            subject = resourceVarName;
            object = QLatin1String("%1");
        }
        if( qbd->flags() & Query::HandleInverseProperties &&
            m_property.inverseProperty().isValid() ) {
            corePattern = QString::fromLatin1("{ %1 %2 %3 . } UNION { %3 %4 %1 . } . ")
                              .arg( subject,
                                    propertyToString( qbd ),
                                    object,
                                    Soprano::Node::resourceToN3( m_property.inverseProperty().uri() ) );
        }
        else {
            corePattern = QString::fromLatin1("%1 %2 %3 . ")
                              .arg( subject,
                                    propertyToString( qbd ),
                                    object );
        }

        if ( m_subTerm.isLiteralTerm() ) {
            //
            // the base of the pattern is always the same: match to resources related to X
            // which has a label that we compare somehow. This label's value will be filled below
            //
            QString v1 = getMainVariableName(qbd);
            QString v2 = qbd->uniqueVarName();
            QString pattern = QString::fromLatin1( "%1%2 %3 %4 . %3 %5 %6 . " )
                              .arg( corePattern.arg(v1),
                                    v1,
                                    v2,
                                    QLatin1String("%1"), // funny way to have a resulting string which takes only one arg
                                    Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::subPropertyOf() ), // using crappy inferencing for now
                                    Soprano::Node::resourceToN3( Soprano::Vocabulary::RDFS::label() ) );

            if ( m_comparator == ComparisonTerm::Equal ) {
                return pattern.arg( Soprano::Node::literalToN3( m_subTerm.toLiteralTerm().value() ) );
            }
            else if ( m_comparator == ComparisonTerm::Contains ) {
                QString v3 = qbd->uniqueVarName();
                QString scoringPattern;
                if( !(qbd->flags()&Query::WithoutScoring) ) {
                    scoringPattern = QString::fromLatin1("OPTION (score %1) ").arg(qbd->createScoringVariable());
                }
                return QString::fromLatin1( "%1%2 bif:contains \"%3\"  %4. " )
                    .arg( pattern.arg(v3),
                          v3,
                          static_cast<const LiteralTermPrivate*>(m_subTerm.toLiteralTerm().d_ptr.constData())->queryText(),
                          scoringPattern );
            }
            else if ( m_comparator == ComparisonTerm::Regexp ) {
                QString v3 = qbd->uniqueVarName();
                return QString::fromLatin1( "%1FILTER(REGEX(STR(%2)), '%3', 'i') . " )
                    .arg( pattern.arg(v3),
                          v3,
                          m_subTerm.toLiteralTerm().value().toString() );
            }
            else {
                kDebug() << QString( "Invalid Term: comparator %1 cannot be used for matching to a resource!" ).arg( comparatorToString( m_comparator ) );
                return QString();
            }
        }
        else if ( m_subTerm.isResourceTerm() ) {
            // ?r <prop> <res>
            return corePattern.arg( m_subTerm.d_ptr->toSparqlGraphPattern( resourceVarName, qbd ) );
        }
        else {
            // ?r <prop> ?v1 . ?v1 ...
            QString v = getMainVariableName(qbd);
            qbd->increaseDepth();
            QString subTermSparql = m_subTerm.d_ptr->toSparqlGraphPattern( v, qbd );
            qbd->decreaseDepth();
            return corePattern.arg(v) + subTermSparql;
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
    // an invalid property will simply match all properties
    // and an invalid subterm is a wildcard, too
    // Thus, a ComparisonTerm is always valid
    return true;
}


//
// Determines the main variable name, i.e. the variable representing the compared value, i.e. the one
// that can be set vie setVariableName.
//
// Thus, the basic scheme is: if a variable name has been set via setVariableName, return that name,
// otherwise create a random new one.
//
// But this method also handles AggregateFunction and sorting. The latter is a bit hacky.
//
// There a quite a few cases that are handled:
//
// 1. No variable name set
// 1.1 no aggregate function set
// 1.1.1 no sorting weight set
//       -> return a new random variable
// 1.1.2 sorting weight set
//       -> determine a new random variable, use it as sorting var (via QueryBuilderData::addOrderVariable),
//          and return it
// 1.2 an aggregate function has been set
// 1.2.1 sorting weight set (no sorting weight is useless and behaves exactly as if no aggregate function was set)
//       -> embed a new random variable in the aggregate expression, add that as sort variable, and
//          return the new variable
// 2. A variable name has been set
// 2.1 no aggregate function set
// 2.1.1 no sorting weight set
//       -> add the variable name as custom variable via QueryBuilderData::addCustomVariable and return the variable name
// 2.1.2 sorting weight set
//       -> add the variable name as sort car via QueryBuilderData::addOrderVariable and return it
// 2.2 an aggregate function has been set
// 2.2.1 no sorting weight set
//       -> Create a new random variable, embed it in the aggregate expression with the set variable name,
//          use that expression as custom variable (this is the hacky part), and return the random one
// 2.2.2 sorting weight set
//       -> Do the same as above only also add the set variable name as sort variable via QueryBuilderData::addOrderVariable
//
QString Nepomuk::Query::ComparisonTermPrivate::getMainVariableName( QueryBuilderData* qbd ) const
{
    QString v;
    QString sortVar;
    if( !m_variableName.isEmpty() ) {
        sortVar = QLatin1String("?") + m_variableName;
        if( m_aggregateFunction == ComparisonTerm::NoAggregateFunction ) {
            v = sortVar;
            qbd->addCustomVariable( v );
        }
        else {
            // this is a bit hacky as far as the method naming in QueryBuilderData is concerned.
            // we add a select statement as a variable name.
            v = qbd->uniqueVarName();
            QString selectVar = QString::fromLatin1( "%1 as ?%2")
                                .arg(varInAggregateFunction(m_aggregateFunction, v),
                                     m_variableName );
            qbd->addCustomVariable( selectVar );
        }
    }
    else {
        v = qbd->uniqueVarName();
        if( m_aggregateFunction == ComparisonTerm::NoAggregateFunction )
            sortVar = v;
        else
            sortVar = varInAggregateFunction(m_aggregateFunction, v);
    }
    if( m_sortWeight != 0 ) {
        qbd->addOrderVariable( sortVar, m_sortWeight, m_sortOrder );
    }
    return v;
}


QString Nepomuk::Query::ComparisonTermPrivate::propertyToString( QueryBuilderData* qbd ) const
{
    if( m_property.isValid() )
        return Soprano::Node::resourceToN3( m_property.uri() );
    else
        return qbd->uniqueVarName();
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


void Nepomuk::Query::ComparisonTerm::setVariableName( const QString& name )
{
    N_D( ComparisonTerm );
    d->m_variableName = name;
}


QString Nepomuk::Query::ComparisonTerm::variableName() const
{
    N_D_CONST( ComparisonTerm );
    return d->m_variableName;
}


void Nepomuk::Query::ComparisonTerm::setAggregateFunction( AggregateFunction function )
{
    N_D( ComparisonTerm );
    d->m_aggregateFunction = function;
}


Nepomuk::Query::ComparisonTerm::AggregateFunction Nepomuk::Query::ComparisonTerm::aggregateFunction() const
{
    N_D_CONST( ComparisonTerm );
    return d->m_aggregateFunction;
}


void Nepomuk::Query::ComparisonTerm::setSortWeight( int weight, Qt::SortOrder sortOrder )
{
    N_D( ComparisonTerm );
    d->m_sortWeight = weight;
    d->m_sortOrder = sortOrder;
}


int Nepomuk::Query::ComparisonTerm::sortWeight() const
{
    N_D_CONST( ComparisonTerm );
    return d->m_sortWeight;
}


Qt::SortOrder Nepomuk::Query::ComparisonTerm::sortOrder() const
{
    N_D_CONST( ComparisonTerm );
    return d->m_sortOrder;
}


bool Nepomuk::Query::ComparisonTerm::isInverted() const
{
    N_D_CONST( ComparisonTerm );
    return d->m_inverted;
}


void Nepomuk::Query::ComparisonTerm::setInverted( bool invert )
{
    N_D( ComparisonTerm );
    d->m_inverted = invert;
}


Nepomuk::Query::ComparisonTerm Nepomuk::Query::ComparisonTerm::inverted() const
{
    ComparisonTerm ct( *this );
    ct.setInverted( !isInverted() );
    return ct;
}
