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
#include "resourcemanager.h"

#include <QtCore/QHash>

#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/RDFS>



Nepomuk::Types::EntityPrivate::EntityPrivate( const QUrl& uri_ )
    : uri( uri_ ),
      available( uri_.isValid() ? -1 : 0 ),
      ancestorsAvailable( uri_.isValid() ? -1 : 0 )
{
}


void Nepomuk::Types::EntityPrivate::init()
{
    if ( available < 0 ) {
        available = load() ? 1 : 0;
    }
}


void Nepomuk::Types::EntityPrivate::initAncestors()
{
    if ( ancestorsAvailable < 0 ) {
        ancestorsAvailable = load() ? 1 : 0;
    }
}


bool Nepomuk::Types::EntityPrivate::load()
{
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?p ?o where { "
                                                                          "graph ?g { <%1> ?p ?o . } . "
                                                                          "?g a <%2> . }")
                                                                  .arg( QString::fromAscii( uri.toEncoded() ) )
                                                                  .arg( Soprano::Vocabulary::NRL::Ontology().toString() ),
                                                                  Soprano::Query::QueryLanguageSparql );
    bool success = false;
    while ( it.next() ) {
        success = true;
        QUrl property = it.binding( "p" ).uri();
        Soprano::Node value = it.binding( "o" );

        if ( property == Soprano::Vocabulary::RDFS::label() ) {
            if ( value.language().isEmpty() ) {
                label = value.toString();
            }
            else {
                l10nLabels[value.language()] = value.toString();
            }
        }

        else if ( property == Soprano::Vocabulary::RDFS::comment() ) {
            if ( value.language().isEmpty() ) {
                comment = value.toString();
            }
            else {
                l10nComments[value.language()] = value.toString();
            }
        }

        else {
            addProperty( property, value );
        }
    }

    return success;
}


bool Nepomuk::Types::EntityPrivate::loadAncestors()
{
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?s ?p where { "
                                                                          "graph ?g { ?s ?p <%1> . } . "
                                                                          "?g a <%2> . }")
                                                                  .arg( QString::fromAscii( uri.toEncoded() ) )
                                                                  .arg( Soprano::Vocabulary::NRL::Ontology().toString() ),
                                                                  Soprano::Query::QueryLanguageSparql );
    bool success = false;
    while ( it.next() ) {
        success = true;
        addAncestorProperty( it.binding( "s" ).uri(), it.binding( "p" ) );
    }

    return success;
}


Nepomuk::Types::Entity::Entity()
{
}


Nepomuk::Types::Entity::Entity( const Entity& other )
{
    d = other.d;
}


Nepomuk::Types::Entity::~Entity()
{
}


Nepomuk::Types::Entity& Nepomuk::Types::Entity::operator=( const Entity& other )
{
    d = other.d;
    return *this;
}


QUrl Nepomuk::Types::Entity::uri() const
{
    return d->uri;
}


QString Nepomuk::Types::Entity::name() const
{
    return d->uri.fragment();
}


QString Nepomuk::Types::Entity::label( const QString& language )
{
    d->init();

    QHash<QString, QString>::const_iterator it = d->l10nLabels.find( language );
    if ( it != d->l10nLabels.constEnd() ) {
        return it.value();
    }
    else {
        return d->label;
    }
}


QString Nepomuk::Types::Entity::comment( const QString& language )
{
    d->init();

    QHash<QString, QString>::const_iterator it = d->l10nComments.find( language );
    if ( it != d->l10nComments.constEnd() ) {
        return it.value();
    }
    else {
        return d->comment;
    }
}


bool Nepomuk::Types::Entity::isValid() const
{
    return d->uri.isValid();
}


bool Nepomuk::Types::Entity::isAvailable()
{
    d->init();
    return d->available == 1;
}


bool Nepomuk::Types::Entity::operator==( const Entity& other )
{
    return d->uri == other.d->uri;
}


bool Nepomuk::Types::Entity::operator!=( const Entity& other )
{
    return d->uri != other.d->uri;
}




// Code for old deprecated Entity class
// -------------------------------------------
Nepomuk::Entity::Entity()
{
    d = new Private();
}

Nepomuk::Entity::Entity( const Entity& other )
{
    d = other.d;
}


Nepomuk::Entity::~Entity()
{
}


Nepomuk::Entity& Nepomuk::Entity::operator=( const Entity& other )
{
    d = other.d;
    return *this;
}


const Nepomuk::Ontology* Nepomuk::Entity::definingOntology() const
{
    return d->ontology;
}


QUrl Nepomuk::Entity::uri() const
{
    return d->uri;
}


QString Nepomuk::Entity::name() const
{
    return d->uri.fragment();
}


QString Nepomuk::Entity::label( const QString& language ) const
{
    QHash<QString, QString>::const_iterator it = d->l10nLabels.find( language );
    if ( it != d->l10nLabels.constEnd() ) {
        return it.value();
    }
    else {
        return d->label;
    }
}


QString Nepomuk::Entity::comment( const QString& language ) const
{
    QHash<QString, QString>::const_iterator it = d->l10nComments.find( language );
    if ( it != d->l10nComments.constEnd() ) {
        return it.value();
    }
    else {
        return d->comment;
    }
}
