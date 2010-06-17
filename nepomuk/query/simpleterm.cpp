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

#include "simpleterm.h"
#include "simpleterm_p.h"

bool Nepomuk::Query::SimpleTermPrivate::equals( const TermPrivate* other ) const
{
    if ( other->m_type == m_type ) {
        return( m_subTerm == static_cast<const SimpleTermPrivate*>( other )->m_subTerm );
    }
    else {
        return false;
    }
}


Nepomuk::Query::SimpleTerm::SimpleTerm( const Term& term )
    : Term( term )
{
}


Nepomuk::Query::SimpleTerm::SimpleTerm( TermPrivate* d )
    : Term( d )
{
}


Nepomuk::Query::SimpleTerm::~SimpleTerm()
{
}

Nepomuk::Query::Term Nepomuk::Query::SimpleTerm::subTerm() const
{
    N_D_CONST( SimpleTerm );
    return d->m_subTerm;
}


void Nepomuk::Query::SimpleTerm::setSubTerm( const Term& term )
{
    N_D( SimpleTerm );
    d->m_subTerm = term;
}
