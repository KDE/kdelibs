/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2009 Sebastian Trueg <trueg@kde.org>
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

#include "resourcefiltermodel.h"
#include "graphwrapper_p.h"
#include "resourcemanager.h"

#include <Soprano/Node>
#include <Soprano/Statement>
#include <Soprano/StatementIterator>
#include <Soprano/NodeIterator>
#include <Soprano/QueryResultIterator>
#include <Soprano/Client/DBusModel>
#include <Soprano/Client/DBusClient>
#include <Soprano/Client/LocalSocketClient>
#include <Soprano/Query/QueryLanguage>
#include <Soprano/Util/DummyModel>
#include <Soprano/Util/MutexModel>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/NAO>

#include <QtCore/QSet>

using namespace Soprano;


class Nepomuk::ResourceFilterModel::Private
{
public:
    GraphWrapper graphWrapper;
};

Nepomuk::ResourceFilterModel::ResourceFilterModel( ResourceManager* manager )
    : FilterModel( 0 ),
      d(new Private())
{
    d->graphWrapper.setManager( manager );
}


Nepomuk::ResourceFilterModel::~ResourceFilterModel()
{
    delete d;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::addStatement( const Statement& statement )
{
    Statement s( statement );
    if( s.context().isEmpty() ) {
        s.setContext( mainContext() );
    }
    return FilterModel::addStatement( s );
}


QUrl Nepomuk::ResourceFilterModel::mainContext()
{
    return d->graphWrapper.currentGraph();
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::updateModificationDate( const QUrl& resource, const QDateTime& date )
{
    Error::ErrorCode c = removeAllStatements( resource, Soprano::Vocabulary::NAO::lastModified(), Soprano::Node() );
    if ( c != Error::ErrorNone )
        return c;
    else
        return addStatement( resource, Soprano::Vocabulary::NAO::lastModified(), LiteralValue( date ), mainContext() );
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::updateProperty( const QUrl& resource, const QUrl& property, const Node& value )
{
    if( !property.isValid() ) {
        setError( "Cannot update invalid property", Error::ErrorInvalidArgument );
        return Error::ErrorInvalidArgument;
    }

    StatementIterator it = listStatements( Statement( resource, property, Node() ) );
    if ( it.next() ) {
        Statement s = it.current();
        it.close();
        if ( s.object() == value ) {
            // nothing to do. Yey!
            return Error::ErrorNone;
        }
        else {
            removeStatement( s );
        }
    }

    // update property
    Error::ErrorCode c = addStatement( resource, property, value, mainContext() );
    if ( c != Error::ErrorNone )
        return updateModificationDate( resource );

    return c;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::updateProperty( const QUrl& resource, const QUrl& property, const QList<Node>& values )
{
    if( !property.isValid() ) {
        setError( "Cannot update invalid property", Error::ErrorInvalidArgument );
        return Error::ErrorInvalidArgument;
    }

    QList<Node> existingValues = listStatements( Statement( resource, property, Node() ) ).iterateObjects().allNodes();

    Error::ErrorCode c = Error::ErrorNone;
    foreach( const Node &node, existingValues.toSet() - values.toSet() ) {
        if ( ( c = removeAllStatements( Statement( resource, property, node ) ) ) != Error::ErrorNone ) {
            return c;
        }
    }

    QSet<Node> newNodes = values.toSet() - existingValues.toSet();
    if ( !newNodes.isEmpty() ) {
        QUrl context = mainContext();
        foreach( const Node &node, newNodes ) {
            if ( ( c = addStatement( Statement( resource, property, node, context ) ) ) != Error::ErrorNone ) {
                return c;
            }
        }

        c = updateModificationDate( resource );
    }

    return c;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::removeProperty( const QUrl& resource, const QUrl& property )
{
    if( !property.isValid() ) {
        setError( "Cannot remove invalid property", Error::ErrorInvalidArgument );
        return Error::ErrorInvalidArgument;
    }

    Soprano::Error::ErrorCode c = removeAllStatements( Statement( resource, property, Node() ) );
    if ( c == Soprano::Error::ErrorNone )
        return updateModificationDate( resource );
    else
        return c;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::ensureResource( const QUrl& resource, const QUrl& type )
{
    if ( !containsAnyStatement( Statement( resource, Soprano::Vocabulary::RDF::type(), type ) ) ) {
         Soprano::Error::ErrorCode c = addStatement( Statement( resource, Soprano::Vocabulary::RDF::type(), type, mainContext() ) );
         if ( c == Soprano::Error::ErrorNone )
             return updateModificationDate( resource );
         else
             return c;
    }
    else {
        clearError();
        return Error::ErrorNone;
    }
}

#include "resourcefiltermodel.moc"
