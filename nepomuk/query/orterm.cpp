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

#include "orterm.h"
#include "orterm_p.h"
#include "querybuilderdata_p.h"

#include <QtCore/QStringList>

QString Nepomuk::Query::OrTermPrivate::toSparqlGraphPattern( const QString& resourceVarName, const TermPrivate* parentTerm, QueryBuilderData* qbd ) const
{
    Q_UNUSED(parentTerm);

    QStringList pattern;

    qbd->pushGroupTerm(this);
    foreach( const Nepomuk::Query::Term &t, m_subTerms ) {
        pattern += t.d_ptr->toSparqlGraphPattern( resourceVarName, this, qbd );
    }
    qbd->popGroupTerm();

    return QLatin1String( "{ " ) + pattern.join( QLatin1String( "} UNION { ") ) + QLatin1String( "} . " );
}


Nepomuk::Query::OrTerm::OrTerm()
    : GroupTerm( new OrTermPrivate() )
{
}


Nepomuk::Query::OrTerm::OrTerm( const OrTerm& term )
    : GroupTerm( term )
{
}


Nepomuk::Query::OrTerm::OrTerm( const Term& term1,
                                const Term& term2,
                                const Term& term3,
                                const Term& term4,
                                const Term& term5,
                                const Term& term6 )
    : GroupTerm( new OrTermPrivate(),
                 term1,
                 term2,
                 term3,
                 term4,
                 term5,
                 term6 )
{
}


Nepomuk::Query::OrTerm::OrTerm( const QList<Term>& terms )
    : GroupTerm( new OrTermPrivate() )
{
    setSubTerms( terms );
}


Nepomuk::Query::OrTerm::~OrTerm()
{
}


Nepomuk::Query::OrTerm& Nepomuk::Query::OrTerm::operator=( const OrTerm& term )
{
    d_ptr = term.d_ptr;
    return *this;
}
