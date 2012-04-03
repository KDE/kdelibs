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

#include "negationterm.h"
#include "negationterm_p.h"
#include "querybuilderdata_p.h"
#include "andterm.h"
#include "andterm_p.h"
#include "comparisonterm.h"

QString Nepomuk::Query::NegationTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, const TermPrivate* parentTerm, const QString &additionalFilters, QueryBuilderData *qbd ) const
{
    //
    // A small optimization: we can negate filters very easily
    //
    if(m_subTerm.isComparisonTerm() && m_subTerm.toComparisonTerm().comparator() == ComparisonTerm::Regexp)  {
        QString term = m_subTerm.d_ptr->toSparqlGraphPattern( resourceVarName, parentTerm, additionalFilters, qbd );
        const int pos = term.indexOf(QLatin1String("FILTER"));
        term.insert(pos+7, QLatin1Char('!'));
        return term;
    }
    else {
        //
        // A negation is expressed via a filter. Since filters can only work on a "real" graph pattern
        // we need to make sure that such a pattern exists. This can be done by searching one in a
        // surrounding AndTerm.
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
        // FIXME: remove the type pattern. Instead perform optimization before which copies the negation into all unions
        bool haveRealTerm = false;
        if( parentTerm && parentTerm->m_type == Term::And ) {
            haveRealTerm = static_cast<const AndTermPrivate*>(parentTerm)->hasRealPattern();
        }

        QString term;
        if( !haveRealTerm ) {
            term += QString::fromLatin1("%1 a %2 . ")
                    .arg( resourceVarName, qbd->uniqueVarName() );
        }

        term += QString( "FILTER NOT EXISTS { %1 } . " )
                .arg( m_subTerm.d_ptr->toSparqlGraphPattern( resourceVarName, this, QString(), qbd ) );

        term += additionalFilters;

        return term;
    }
}


Nepomuk::Query::NegationTerm::NegationTerm()
    : SimpleTerm( new NegationTermPrivate() )
{
}


Nepomuk::Query::NegationTerm::NegationTerm( const NegationTerm& term )
    : SimpleTerm( term )
{
}


Nepomuk::Query::NegationTerm::~NegationTerm()
{
}


Nepomuk::Query::NegationTerm& Nepomuk::Query::NegationTerm::operator=( const NegationTerm& term )
{
    d_ptr = term.d_ptr;
    return *this;
}


Nepomuk::Query::Term Nepomuk::Query::NegationTerm::negateTerm( const Term& term )
{
    if ( term.isNegationTerm() ) {
        return term.toNegationTerm().subTerm();
    }
    else {
        NegationTerm nt;
        nt.setSubTerm( term );
        return nt;
    }
}
