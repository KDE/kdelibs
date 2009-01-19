/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2007-2009 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "tag.h"

#include "tools.h"
#include "variant.h"
#include "resourcemanager.h"
#include "resource.h"

#include <Soprano/Vocabulary/NAO>


Nepomuk::Tag::Tag()
    : Resource()
{
}


Nepomuk::Tag::Tag( ResourceManager* manager )
    : Resource( manager )
{
}


Nepomuk::Tag::Tag( const Tag& res )
    : Resource( res )
{
}


Nepomuk::Tag::Tag( const Nepomuk::Resource& res )
    : Resource( res )
{
}


Nepomuk::Tag::Tag( const QString& uri )
    : Resource( uri, Soprano::Vocabulary::NAO::Tag() )
{
}


Nepomuk::Tag::Tag( const QString& uri, ResourceManager* manager )
    : Resource( uri, Soprano::Vocabulary::NAO::Tag(), manager )
{
}


Nepomuk::Tag::Tag( const QUrl& uri )
    : Resource( uri, Soprano::Vocabulary::NAO::Tag() )
{
}


Nepomuk::Tag::Tag( const QUrl& uri, ResourceManager* manager )
    : Resource( uri, Soprano::Vocabulary::NAO::Tag(), manager )
{
}


Nepomuk::Tag::Tag( const QString& uri, const QUrl& type )
    : Resource( uri, type )
{
}


Nepomuk::Tag::Tag( const QString& uri, const QUrl& type, ResourceManager* manager )
    : Resource( uri, type, manager )
{
}


Nepomuk::Tag::Tag( const QUrl& uri, const QUrl& type )
    : Resource( uri, type )
{
}


Nepomuk::Tag::Tag( const QUrl& uri, const QUrl& type, ResourceManager* manager )
    : Resource( uri, type, manager )
{
}


Nepomuk::Tag::~Tag()
{
}


Nepomuk::Tag& Nepomuk::Tag::operator=( const Tag& res )
{
    Resource::operator=( res );
    return *this;
}


QString Nepomuk::Tag::resourceTypeUri()
{
    return Soprano::Vocabulary::NAO::Tag().toString();
}


QList<Nepomuk::Resource> Nepomuk::Tag::tagOf() const
{
    return convertResourceList<Resource>( manager()->allResourcesWithProperty( Soprano::Vocabulary::NAO::hasTag(), *this ) );
}


// static
QList<Nepomuk::Tag> Nepomuk::Tag::allTags()
{
    return Nepomuk::convertResourceList<Tag>( ResourceManager::instance()->allResourcesOfType( Soprano::Vocabulary::NAO::Tag() ) );
}
