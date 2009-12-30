/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2009 Sebastian Trueg <trueg@kde.org>

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

// static
Nepomuk::Query::Term Nepomuk::Query::QueryPrivate::optimizeTerm( const Nepomuk::Query::Term& term )
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
            Nepomuk::Query::Term ot = optimizeTerm( t );
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

    default:
        return term;
    }
}

