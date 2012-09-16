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

#include "resourcetypeterm.h"
#include "resourcetypeterm_p.h"
#include "querybuilderdata_p.h"

#include <Soprano/Node>
#include <Soprano/Vocabulary/RDFS>


bool Nepomuk::Query::ResourceTypeTermPrivate::equals( const TermPrivate* other ) const
{
    if ( other->m_type == m_type ) {
        const ResourceTypeTermPrivate* rtp = static_cast<const ResourceTypeTermPrivate*>( other );
        return rtp->m_types == m_types;
    }
    else {
        return false;
    }
}


QString Nepomuk::Query::ResourceTypeTermPrivate::toSparqlGraphPattern( const QString& resName, const TermPrivate* parentTerm, const QString &additionalFilters, QueryBuilderData *qbd ) const
{
    Q_UNUSED(parentTerm);

    // we are using the crappy inferencing provided by the nepomuk ontology service where
    // each class is also a subclass of itself.
    if(m_types.count() == 1) {
        return QString::fromLatin1("%1 a %2 . %3")
                .arg( resName,
                      Soprano::Node::resourceToN3( m_types.begin()->uri() ),
                      additionalFilters );
    }
    else {
        QStringList typeN3s;
        foreach(const Types::Class& type, m_types) {
            typeN3s.append(Soprano::Node::resourceToN3(type.uri()));
        }

        return QString::fromLatin1("%1 a %2 . FILTER(%2 in (%3)) . %4")
                .arg( resName,
                      qbd->uniqueVarName(),
                      typeN3s.join(QLatin1String(",")),
                      additionalFilters);
    }
}


Nepomuk::Query::ResourceTypeTerm::ResourceTypeTerm( const ResourceTypeTerm& term )
    : Term( term )
{
}


Nepomuk::Query::ResourceTypeTerm::ResourceTypeTerm( const Nepomuk::Types::Class& type )
    : Term( new ResourceTypeTermPrivate() )
{
    setType( type );
}


Nepomuk::Query::ResourceTypeTerm::~ResourceTypeTerm()
{
}


Nepomuk::Query::ResourceTypeTerm& Nepomuk::Query::ResourceTypeTerm::operator=( const ResourceTypeTerm& term )
{
    d_ptr = term.d_ptr;
    return *this;
}


Nepomuk::Types::Class Nepomuk::Query::ResourceTypeTerm::type() const
{
    N_D_CONST( ResourceTypeTerm );
    if(!d->m_types.isEmpty())
        return *d->m_types.begin();
    else
        return Types::Class();
}


void Nepomuk::Query::ResourceTypeTerm::setType( const Nepomuk::Types::Class& type )
{
    N_D( ResourceTypeTerm );
    d->m_types.clear();
    if(type.isValid())
        d->m_types.insert(type);
}
