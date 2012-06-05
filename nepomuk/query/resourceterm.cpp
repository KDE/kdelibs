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

#include "resourceterm.h"
#include "resourceterm_p.h"
#include "querybuilderdata_p.h"
#include "andterm_p.h"

#include <Soprano/Node>


bool Nepomuk::Query::ResourceTermPrivate::equals( const TermPrivate* other ) const
{
    if ( other->m_type == m_type ) {
        const ResourceTermPrivate* rtp = static_cast<const ResourceTermPrivate*>( other );
        return rtp->m_resource == m_resource;
    }
    else {
        return false;
    }
}


QString Nepomuk::Query::ResourceTermPrivate::toSparqlGraphPattern( const QString& varName, const TermPrivate* parentTerm, const QString& additionalFilters, QueryBuilderData* qbd ) const
{
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
    bool haveRealTerm = false;
    if( parentTerm && parentTerm->m_type == Term::And ) {
        haveRealTerm = static_cast<const AndTermPrivate*>(parentTerm)->hasRealPattern();
    }

    QString term;
    if( !haveRealTerm ) {
        term += QString::fromLatin1("%1 a %2 . ")
                .arg( varName, qbd->uniqueVarName() );
    }

    term += QString::fromLatin1("FILTER(%1=%2) . ")
            .arg( varName,
                  Soprano::Node::resourceToN3( m_resource.resourceUri() ) );

    term += additionalFilters;

    return term;
}


Nepomuk::Query::ResourceTerm::ResourceTerm( const ResourceTerm& term )
    : Term( term )
{
}


Nepomuk::Query::ResourceTerm::ResourceTerm( const Nepomuk::Resource& resource )
    : Term( new ResourceTermPrivate() )
{
    setResource( resource );
}


Nepomuk::Query::ResourceTerm::~ResourceTerm()
{
}


Nepomuk::Query::ResourceTerm& Nepomuk::Query::ResourceTerm::operator=( const ResourceTerm& term )
{
    d_ptr = term.d_ptr;
    return *this;
}


Nepomuk::Resource Nepomuk::Query::ResourceTerm::resource() const
{
    N_D_CONST( ResourceTerm );
    return d->m_resource;
}


void Nepomuk::Query::ResourceTerm::setResource( const Nepomuk::Resource& resource )
{
    N_D( ResourceTerm );
    d->m_resource = resource;
}
