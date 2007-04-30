/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "entity.h"
#include "entity_p.h"

#include <QHash>


Konto::Entity::Entity()
{
    d = new Private();
}

Konto::Entity::Entity( const Entity& other )
{
    d = other.d;
}


Konto::Entity::~Entity()
{
}


Konto::Entity& Konto::Entity::operator=( const Entity& other )
{
    d = other.d;
    return *this;
}


const Konto::Ontology* Konto::Entity::definingOntology() const
{
    return d->ontology;
}


QUrl Konto::Entity::uri() const
{
    return d->uri;
}


QString Konto::Entity::name() const
{
    return d->uri.fragment();
}


QString Konto::Entity::label( const QString& language ) const
{
    QHash<QString, QString>::const_iterator it = d->l10nLabels.find( language );
    if ( it != d->l10nLabels.constEnd() ) {
        return it.value();
    }
    else {
        return d->label;
    }
}


QString Konto::Entity::comment( const QString& language ) const
{
    QHash<QString, QString>::const_iterator it = d->l10nComments.find( language );
    if ( it != d->l10nComments.constEnd() ) {
        return it.value();
    }
    else {
        return d->comment;
    }
}
