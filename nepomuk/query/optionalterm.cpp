/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2010 Sebastian Trueg <trueg@kde.org>

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

#include "optionalterm.h"
#include "optionalterm_p.h"
#include "querybuilderdata_p.h"

QString Nepomuk::Query::OptionalTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, QueryBuilderData* qbd ) const
{
    return QString( "OPTIONAL { %1} " )
        .arg( m_subTerm.d_ptr->toSparqlGraphPattern( resourceVarName, qbd ) );
}


Nepomuk::Query::OptionalTerm::OptionalTerm()
    : SimpleTerm( new OptionalTermPrivate() )
{
}


Nepomuk::Query::OptionalTerm::OptionalTerm( const OptionalTerm& term )
    : SimpleTerm( term )
{
}


Nepomuk::Query::OptionalTerm::~OptionalTerm()
{
}


Nepomuk::Query::OptionalTerm& Nepomuk::Query::OptionalTerm::operator=( const OptionalTerm& term )
{
    d_ptr = term.d_ptr;
    return *this;
}


Nepomuk::Query::Term Nepomuk::Query::OptionalTerm::optionalizeTerm( const Term& term )
{
    if ( !term.isOptionalTerm() ) {
        OptionalTerm ot;
        ot.setSubTerm(term);
        return ot;
    }
    else {
        return term;
    }
}
