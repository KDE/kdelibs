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

#include "groupterm.h"
#include "groupterm_p.h"

#include <QtCore/QStringList>

bool Nepomuk::Query::GroupTermPrivate::equals( const TermPrivate* other ) const
{
    if ( other->m_type == m_type ) {
        // TODO: would be nicer to compare sorted lists or something
        const GroupTermPrivate* gtp = static_cast<const GroupTermPrivate*>( other );
        return gtp->m_subTerms == m_subTerms;
    }
    else {
        return false;
    }
}


bool Nepomuk::Query::GroupTermPrivate::isValid() const
{
    if ( m_subTerms.count() ) {
        foreach( const Term& t, m_subTerms ) {
            if ( !t.isValid() )
                return false;
        }
        return true;
    }
    else {
        return false;
    }
}


Nepomuk::Query::GroupTerm::GroupTerm( const Term& term )
    : Term( term )
{
}


Nepomuk::Query::GroupTerm::GroupTerm( TermPrivate* d )
    : Term( d )
{
}


Nepomuk::Query::GroupTerm::GroupTerm( TermPrivate* d,
                                      const Term& term1,
                                      const Term& term2,
                                      const Term& term3,
                                      const Term& term4,
                                      const Term& term5,
                                      const Term& term6 )
    : Term( d )
{
    addSubTerm( term1 );
    addSubTerm( term2 );
    if ( term3.isValid() )
        addSubTerm( term3 );
    if ( term4.isValid() )
        addSubTerm( term4 );
    if ( term5.isValid() )
        addSubTerm( term5 );
    if ( term6.isValid() )
        addSubTerm( term6 );
}


Nepomuk::Query::GroupTerm::~GroupTerm()
{
}


QList<Nepomuk::Query::Term> Nepomuk::Query::GroupTerm::subTerms() const
{
    N_D_CONST( GroupTerm );
    return d->m_subTerms;
}


void Nepomuk::Query::GroupTerm::setSubTerms( const QList<Term>& terms )
{
    N_D( GroupTerm );
    d->m_subTerms = terms;
}


void Nepomuk::Query::GroupTerm::addSubTerm( const Term& term )
{
    N_D( GroupTerm );
    d->m_subTerms << term;
}
