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

#include "facet.h"
#include "property.h"
#include "comparisonterm.h"
#include "literalterm.h"

#include <QtCore/QSharedData>


class Nepomuk::Query::Facet::FacetPrivate : public QSharedData
{
public:
    FacetPrivate()
        : m_isExclusive(true) {
    }

    bool haveNoSelectionTerm() const {
        return !m_noSelectionTitle.isEmpty();
    }

    QString noSelectionTitle() const {
        if( m_noSelectionTitle.isEmpty() )
            return i18n("No restriction");
        else
            return m_noSelectionTitle;
    }

    bool m_isExclusive;

    QString m_title;
    QString m_noSelectionTitle;

    QList<Nepomuk::Query::Term> m_terms;
    QList<QString> m_titles;

    Nepomuk::Types::Property m_rangeProperty;
    QString m_rangeTitle;

    QVariant m_rangeStart;
    QVariant m_rangeEnd;
};


Nepomuk::Query::Facet::Facet()
    : d(new FacetPrivate())
{
}


Nepomuk::Query::Facet::Facet( const Facet& other )
{
    d = other.d;
}


Nepomuk::Query::Facet::~Facet()
{
}


Nepomuk::Query::Facet& Nepomuk::Query::Facet::operator=( const Facet& other )
{
    d = other.d;
    return *this;
}


QString Nepomuk::Query::Facet::title() const
{
    return d->m_title;
}


bool Nepomuk::Query::Facet::exclusive() const
{
    return d->m_isExclusive;
}


QString Nepomuk::Query::Facet::rangeTitle() const
{
    if( d->m_rangeTitle.isEmpty() )
        return i18n("Custom range");
    else
        return d->m_rangeTitle;
}


Nepomuk::Types::Property Nepomuk::Query::Facet::rangeProperty() const
{
    return d->m_rangeProperty;
}


void Nepomuk::Query::Facet::setTitle( const QString& title )
{
    d->m_title = title;
}


void Nepomuk::Query::Facet::setExclusive( bool exclusive )
{
    d->m_isExclusive = exclusive;
}


void Nepomuk::Query::Facet::setNoSelectionTitle( const QString& title )
{
    d->m_noSelectionTitle = title;
}


void Nepomuk::Query::Facet::setRangeProperty( const Nepomuk::Types::Property& prop )
{
    d->m_rangeProperty = prop;
}


void Nepomuk::Query::Facet::setRangeTitle( const QString& title )
{
    d->m_rangeTitle = title;
}


void Nepomuk::Query::Facet::setRange( const QVariant& start, const QVariant& end )
{
    d->m_rangeStart = start;
    d->m_rangeEnd = end;
}


Nepomuk::Query::Term Nepomuk::Query::Facet::createRangeTerm() const
{
    if( rangeProperty().isValid() ) {
        return( rangeProperty() >= LiteralTerm( d->m_rangeStart ) &&
                rangeProperty() <= LiteralTerm( d->m_rangeEnd ) );
    }
    else {
        return Term();
    }
}


void Nepomuk::Query::Facet::addTerm( const Term& term, const QString& title )
{
    d->m_terms.append(term);
    d->m_titles.append(title);
}


void Nepomuk::Query::Facet::clear()
{
    d->m_terms.clear();
    d->m_titles.clear();
}


int Nepomuk::Query::Facet::count() const
{
    int c = d->m_terms.count();
    if( d->haveNoSelectionTerm() ) {
        ++c;
    }
    return c;
}


Nepomuk::Query::Term Nepomuk::Query::Facet::termAt( int i ) const
{
    if( d->haveNoSelectionTerm() ) {
        if( i == 0 )
            return Term();
        --i;
    }

    return d->m_terms.at(i);
}


QString Nepomuk::Query::Facet::titleAt( int i ) const
{
    if( d->haveNoSelectionTerm() ) {
        if( i == 0 )
            return d->noSelectionTitle();
        --i;
    }

    return d->m_titles.at(i);
}


QString Nepomuk::Query::Facet::termTitle( const Term& term ) const
{
    return d->m_titles[d->m_terms.indexOf(term)];
}


QList<Nepomuk::Query::Term> Nepomuk::Query::Facet::termList() const
{
    QList<Nepomuk::Query::Term> terms = d->m_terms;
    if( d->haveNoSelectionTerm() ) {
        terms.prepend( Term() );
    }
    return terms;
}


bool Nepomuk::Query::Facet::operator==( const Facet& other ) const
{
}


bool Nepomuk::Query::Facet::operator!=( const Facet& other ) const
{
}


uint Nepomuk::Query::qHash( const Facet& facet )
{
}
