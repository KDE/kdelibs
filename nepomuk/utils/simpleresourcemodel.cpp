/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008-2010 Sebastian Trueg <trueg@kde.org>

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

#include "simpleresourcemodel.h"

#include <QtCore/QUrl>
#include <QtCore/QList>

#include "resource.h"
#include "result.h"

#include "kdebug.h"
#include "kurl.h"


class Nepomuk::Utils::SimpleResourceModel::Private
{
public:
    QList<Nepomuk::Resource> resources;
};


Nepomuk::Utils::SimpleResourceModel::SimpleResourceModel( QObject* parent )
    : ResourceModel( parent ),
      d( new Private() )
{
}


Nepomuk::Utils::SimpleResourceModel::~SimpleResourceModel()
{
    delete d;
}


QModelIndex Nepomuk::Utils::SimpleResourceModel::indexForResource( const Resource& res ) const
{
    Q_ASSERT( res.isValid() );
    // FIXME: performance
    int i = 0;
    QList<Nepomuk::Resource>::const_iterator end = d->resources.constEnd();
    for ( QList<Nepomuk::Resource>::const_iterator it = d->resources.constBegin(); it != end; ++it ) {
        if ( *it == res ) {
            return index( i, 0 );
        }
        ++i;
    }

    return QModelIndex();
}


Nepomuk::Resource Nepomuk::Utils::SimpleResourceModel::resourceForIndex( const QModelIndex& index ) const
{
    if ( index.isValid() && index.row() < d->resources.count() ) {
        return d->resources[index.row()];
    }
    else {
        return Resource();
    }
}


int Nepomuk::Utils::SimpleResourceModel::rowCount( const QModelIndex& parent ) const
{
    if ( parent.isValid() ) {
        return 0;
    }
    else {
        return d->resources.count();
    }
}


QModelIndex Nepomuk::Utils::SimpleResourceModel::index( int row, int column, const QModelIndex& parent ) const
{
    if ( !parent.isValid() && row < d->resources.count() ) {
        return createIndex( row, column, 0 );
    }
    else {
        return QModelIndex();
    }
}


bool Nepomuk::Utils::SimpleResourceModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if( count < 1 || row < 0 || (row + count) > d->resources.size() || parent.isValid() )
        return false;

    beginRemoveRows( parent, row, row + count -1 );

    QList<Resource>::iterator begin, end;
    begin = end = d->resources.begin();
    begin += row;
    end += row + count;
    d->resources.erase( begin, end );

    endRemoveRows();
    return true;
}


void Nepomuk::Utils::SimpleResourceModel::setResources( const QList<Nepomuk::Resource>& resources )
{
    d->resources = resources;
    reset();
}


void Nepomuk::Utils::SimpleResourceModel::addResources( const QList<Nepomuk::Resource>& resources )
{
    if(!resources.isEmpty()) {
        beginInsertRows( QModelIndex(), d->resources.count(), d->resources.count() + resources.count() - 1 );
        d->resources << resources;
        endInsertRows();
    }
}


void Nepomuk::Utils::SimpleResourceModel::addResource( const Nepomuk::Resource& resource )
{
    addResources( QList<Resource>() << resource );
}


void Nepomuk::Utils::SimpleResourceModel::setResults( const QList<Nepomuk::Query::Result>& results)
{
    clear();
    addResults( results );
}

void Nepomuk::Utils::SimpleResourceModel::addResults( const QList<Nepomuk::Query::Result>& results )
{
    Q_FOREACH( const Query::Result& result, results ) {
        addResource( result.resource() );
    }
}

void Nepomuk::Utils::SimpleResourceModel::addResult( const Nepomuk::Query::Result result )
{
    addResource( result.resource() );
}


void Nepomuk::Utils::SimpleResourceModel::clear()
{
    d->resources.clear();
    reset();
}

#include "simpleresourcemodel.moc"
