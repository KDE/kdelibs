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

#include "query_p.h"

#include "groupterm.h"
#include "andterm.h"
#include "orterm.h"
#include "comparisonterm.h"
#include "negationterm.h"
#include "optionalterm.h"

#include <Soprano/Vocabulary/RDF>


namespace {
Nepomuk::Query::Term flattenGroupTerms( const Nepomuk::Query::Term& term )
{
    switch( term.type() ) {
    case Nepomuk::Query::Term::And:
    case Nepomuk::Query::Term::Or: {
        QList<Nepomuk::Query::Term> subTerms = static_cast<const Nepomuk::Query::GroupTerm&>( term ).subTerms();
        QList<Nepomuk::Query::Term> newSubTerms;
        QList<Nepomuk::Query::Term>::const_iterator end( subTerms.constEnd() );
        for ( QList<Nepomuk::Query::Term>::const_iterator it = subTerms.constBegin();
              it != end; ++it ) {
            const Nepomuk::Query::Term& t = *it;
            Nepomuk::Query::Term ot = flattenGroupTerms( t );
            if ( ot.isValid() ) {
                if ( ot.type() == term.type() ) {
                    newSubTerms += static_cast<const Nepomuk::Query::GroupTerm&>( ot ).subTerms();
                }
                else {
                    newSubTerms += ot;
                }
            }
        }
        if ( newSubTerms.count() == 0 )
            return Nepomuk::Query::Term();
        else if ( newSubTerms.count() == 1 )
            return newSubTerms.first();
        else if ( term.isAndTerm() )
            return Nepomuk::Query::AndTerm( newSubTerms );
        else
            return Nepomuk::Query::OrTerm( newSubTerms );
    }

    case Nepomuk::Query::Term::Negation: {
        Nepomuk::Query::NegationTerm nt = term.toNegationTerm();
        // a negation in a negation
        if( nt.subTerm().isNegationTerm() )
            return flattenGroupTerms( nt.subTerm().toNegationTerm().subTerm() );
        else
            return Nepomuk::Query::NegationTerm::negateTerm( flattenGroupTerms( nt.subTerm() ) );
    }

    case Nepomuk::Query::Term::Optional: {
        Nepomuk::Query::OptionalTerm ot = term.toOptionalTerm();
        // remove duplicate optional terms
        if( ot.subTerm().isOptionalTerm() )
            return flattenGroupTerms( ot.subTerm() );
        else
            return Nepomuk::Query::OptionalTerm::optionalizeTerm( flattenGroupTerms( ot.subTerm() ) );
    }

    case Nepomuk::Query::Term::Comparison: {
        Nepomuk::Query::ComparisonTerm ct( term.toComparisonTerm() );
        ct.setSubTerm( flattenGroupTerms( ct.subTerm() ) );
        return ct;
    }

    default:
        return term;
    }
}


Nepomuk::Query::Term fixFilters( const Nepomuk::Query::Term& term )
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
                newAndTerm.addSubTerm( fixFilters( term ) );
            }
        }
        if( haveNegationTerm && !haveRealTerm ) {
            newAndTerm.addSubTerm( Nepomuk::Query::ComparisonTerm( Soprano::Vocabulary::RDF::type(), Nepomuk::Query::Term() ) );
        }
        return newAndTerm;
    }

    case Nepomuk::Query::Term::Or: {
        // call fixFilters on all subterms
        QList<Nepomuk::Query::Term> subTerms = term.toOrTerm().subTerms();
        QList<Nepomuk::Query::Term> newSubTerms;
        Q_FOREACH( const Nepomuk::Query::Term& term, subTerms ) {
            newSubTerms.append( fixFilters( term ) );
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
        return Nepomuk::Query::OptionalTerm::optionalizeTerm( fixFilters( term.toOptionalTerm().subTerm() ) );
    }

    case Nepomuk::Query::Term::Comparison: {
        Nepomuk::Query::ComparisonTerm ct = term.toComparisonTerm();
        ct.setSubTerm( fixFilters( ct.subTerm() ) );
        return ct;
    }

    default:
        return term;
    }
}
}


// static
Nepomuk::Query::Term Nepomuk::Query::QueryPrivate::optimizeTerm( const Nepomuk::Query::Term& term )
{
    return fixFilters( flattenGroupTerms( term ) );
}
