/* This file is part of the Nepomuk-KDE libraries
    Copyright (c) 2007-2009 Sebastian Trueg <trueg@kde.org>

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
#include <QtCore/QMutexLocker>

#include <Soprano/QueryResultIterator>
#include <Soprano/Model>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/NAO>
#include <Soprano/Vocabulary/RDFS>

#include <kicon.h>


Nepomuk::Types::EntityPrivate::EntityPrivate( const QUrl& uri_ )
    : mutex(QMutex::Recursive),
      uri( uri_ ),
      available( uri_.isValid() ? -1 : 0 ),
      ancestorsAvailable( uri_.isValid() ? -1 : 0 )
{
}


void Nepomuk::Types::EntityPrivate::init()
{
    QMutexLocker lock( &mutex );

    if ( available < 0 ) {
        available = load() ? 1 : 0;
    }
}


void Nepomuk::Types::EntityPrivate::initAncestors()
{
    QMutexLocker lock( &mutex );

    if ( ancestorsAvailable < 0 ) {
        ancestorsAvailable = loadAncestors() ? 1 : 0;
    }
}


bool Nepomuk::Types::EntityPrivate::load()
{
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?p ?o where { "
                                                                          "graph ?g { <%1> ?p ?o . } . "
                                                                          "{ ?g a <%2> . } UNION { ?g a <%3> . } . }")
                                                                  .arg( QString::fromAscii( uri.toEncoded() ) )
                                                                  .arg( Soprano::Vocabulary::NRL::Ontology().toString() )
                                                                  .arg( Soprano::Vocabulary::NRL::KnowledgeBase().toString() ),
                                                                  Soprano::Query::QueryLanguageSparql );
    while ( it.next() ) {
        QUrl property = it.binding( "p" ).uri();
        Soprano::Node value = it.binding( "o" );

        if ( property == Soprano::Vocabulary::RDFS::label() ) {
            if ( value.language().isEmpty() ) {
                label = value.toString();
            }
            else if( value.language() == KGlobal::locale()->language() ) {
                l10nLabel = value.toString();
            }
        }

        else if ( property == Soprano::Vocabulary::RDFS::comment() ) {
            if ( value.language().isEmpty() ) {
                comment = value.toString();
            }
            else if( value.language() == KGlobal::locale()->language() ) {
                l10nComment = value.toString();
            }
        }

        else if ( property == Soprano::Vocabulary::NAO::hasSymbol() ) {
            icon = KIcon( value.toString() );
        }

        else {
            addProperty( property, value );
        }
    }

    return !it.lastError();
}


bool Nepomuk::Types::EntityPrivate::loadAncestors()
{
    Soprano::QueryResultIterator it
        = ResourceManager::instance()->mainModel()->executeQuery( QString("select ?s ?p where { "
                                                                          "graph ?g { ?s ?p <%1> . } . "
                                                                          "{ ?g a <%2> . } UNION { ?g a <%3> . } . }")
                                                                  .arg( QString::fromAscii( uri.toEncoded() ) )
                                                                  .arg( Soprano::Vocabulary::NRL::Ontology().toString() )
                                                                  .arg( Soprano::Vocabulary::NRL::KnowledgeBase().toString() ),
                                                                  Soprano::Query::QueryLanguageSparql );
    while ( it.next() ) {
        addAncestorProperty( it.binding( "s" ).uri(), it.binding( "p" ).uri() );
    }

    return !it.lastError();
}



void Nepomuk::Types::EntityPrivate::reset( bool )
{
    QMutexLocker lock( &mutex );

    label.truncate(0);
    comment.truncate(0);
    l10nLabel.truncate(0);
    l10nComment.truncate(0);;

    icon = QIcon();

    available = -1;
    ancestorsAvailable = -1;
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
    return d ? d->uri : QUrl();
}


QString Nepomuk::Types::Entity::name() const
{
    return d ? (d->uri.fragment().isEmpty() ? d->uri.toString().section('/',-1) : d->uri.fragment() ) : QString();
}


QString Nepomuk::Types::Entity::label( const QString& language )
{
    if ( d ) {
        d->init();

        if ( language == KGlobal::locale()->language() &&
             !d->l10nLabel.isEmpty() ) {
            return d->l10nLabel;
        }
        else if( !d->label.isEmpty() ) {
            return d->label;
        }
        else {
            return name();
        }
    }
    else {
        return QString();
    }
}


QString Nepomuk::Types::Entity::label( const QString& language ) const
{
    return const_cast<Entity*>(this)->label( language );
}


QString Nepomuk::Types::Entity::comment( const QString& language )
{
    if ( d ) {
        d->init();

        if ( language == KGlobal::locale()->language() &&
             !d->l10nComment.isEmpty() ) {
            return d->l10nComment;
        }
        else {
            return d->comment;
        }
    }
    else {
        return QString();
    }
}


QString Nepomuk::Types::Entity::comment( const QString& language ) const
{
    return const_cast<Entity*>(this)->comment( language );
}


QIcon Nepomuk::Types::Entity::icon()
{
    if ( d ) {
        d->init();

        return d->icon;
    }
    else {
        return QIcon();
    }
}


QIcon Nepomuk::Types::Entity::icon() const
{
    return const_cast<Entity*>(this)->icon();
}


bool Nepomuk::Types::Entity::isValid() const
{
    return d ? d->uri.isValid() : false;
}


bool Nepomuk::Types::Entity::isAvailable()
{
    if ( d ) {
        d->init();
        return d->available == 1;
    }
    else {
        return false;
    }
}


bool Nepomuk::Types::Entity::isAvailable() const
{
    return const_cast<Entity*>(this)->isAvailable();
}


void Nepomuk::Types::Entity::reset( bool recursive )
{
    d->reset( recursive );
}


bool Nepomuk::Types::Entity::operator==( const Entity& other ) const
{
    // since we use one instace cache we can improve comparation operations
    // intensly by not comparing URLs but pointers.
    return( d.constData() == other.d.constData() );
}


bool Nepomuk::Types::Entity::operator==( const QUrl& other ) const
{
    // since we use one instace cache we can improve comparation operations
    // intensly by not comparing URLs but pointers.
    return( d.constData()->uri == other );
}


bool Nepomuk::Types::Entity::operator!=( const Entity& other ) const
{
    // since we use one instace cache we can improve comparation operations
    // intensly by not comparing URLs but pointers.
    return( d.constData() != other.d.constData() );
}


bool Nepomuk::Types::Entity::operator!=( const QUrl& other ) const
{
    // since we use one instace cache we can improve comparation operations
    // intensly by not comparing URLs but pointers.
    return( d.constData()->uri != other );
}



#ifndef DISABLE_NEPOMUK_LEGACY

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

#endif // DISABLE_NEPOMUK_LEGACY

