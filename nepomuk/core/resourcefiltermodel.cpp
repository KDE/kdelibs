
/*
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
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
#include "resourcemanager.h"

#include <Soprano/Statement>
#include <Soprano/StatementIterator>
#include <Soprano/Node>
#include <Soprano/NodeIterator>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/NRL>
#include <Soprano/Vocabulary/NAO>

#include <QtCore/QSet>
#include <QtCore/QDateTime>

#include <kdebug.h>


// IDEA: Remember the last used named graph for N seconds and reuse it.

using namespace Soprano;


uint Soprano::qHash( const Soprano::Node& node )
{
    return qHash( node.toString() );
}

#ifndef Q_OS_WIN
// @deprecated: just for keeping binary compatibility
uint qHash( const Soprano::Node& node )
{
    return Soprano::qHash( node );
}
#endif

class Nepomuk::ResourceFilterModel::Private
{
public:
};


Nepomuk::ResourceFilterModel::ResourceFilterModel( Soprano::Model* model )
    : Soprano::FilterModel( model ),
      d( 0 )
{
}


Nepomuk::ResourceFilterModel::~ResourceFilterModel()
{
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::updateProperty( const QUrl& resource, const QUrl& property, const Node& value )
{
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

    // FIXME: provedence data!
    QUrl newContext = ResourceManager::instance()->generateUniqueUri();
    Error::ErrorCode c = addStatement( Statement( resource, property, value, newContext ) );
    if ( c == Error::ErrorNone ) {
        c = addStatement( Statement( newContext, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::InstanceBase() ) );
        c = addStatement( Statement( newContext, Soprano::Vocabulary::NAO::created(), LiteralValue( QDateTime::currentDateTime() ) ) );
    }
    return c;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::updateProperty( const QUrl& resource, const QUrl& property, const QList<Node>& values )
{
    QList<Node> existingValues = listStatements( Statement( resource, property, Node() ) ).iterateObjects().allNodes();

    Error::ErrorCode c = Error::ErrorNone;
    foreach( const Node &node, existingValues.toSet() - values.toSet() ) {
        if ( ( c = removeAllStatements( Statement( resource, property, node ) ) ) != Error::ErrorNone ) {
            return c;
        }
    }

    QSet<Node> newNodes = values.toSet() - existingValues.toSet();
    if ( !newNodes.isEmpty() ) {
        QUrl newContext = ResourceManager::instance()->generateUniqueUri();
        foreach( const Node &node, newNodes ) {
            if ( ( c = addStatement( Statement( resource, property, node, newContext ) ) ) != Error::ErrorNone ) {
                return c;
            }
        }
        if ( ( c = addStatement( Statement( newContext, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::InstanceBase() ) ) ) != Error::ErrorNone ) {
            return c;
        }

        // provedence data!
        c = addStatement( Statement( newContext, Soprano::Vocabulary::NAO::created(), LiteralValue( QDateTime::currentDateTime() ) ) );
    }

    return c;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::removeProperty( const QUrl& resource, const QUrl& property )
{
    // remove the data
    Soprano::Error::ErrorCode c = removeAllStatements( Statement( resource, property, Node() ) );
    if ( c != Soprano::Error::ErrorNone ) {
        return c;
    }

    return Error::ErrorNone;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::ensureResource( const QUrl& resource, const QUrl& type )
{
    // FIXME: graph!
    if ( !containsAnyStatement( Statement( resource, Soprano::Vocabulary::RDF::type(), type ) ) ) {
        return addStatement( Statement( resource, Soprano::Vocabulary::RDF::type(), type ) );
    }
    else {
        clearError();
        return Error::ErrorNone;
    }
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::removeStatement( const Statement &statement )
{
    Soprano::Error::ErrorCode c = FilterModel::removeStatement( statement );
    if ( c == Error::ErrorNone && !statement.context().isEmpty() ) {
        return removeGraphIfEmpty( statement.context() );
    }
    else {
        return c;
    }
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::removeAllStatements( const Statement &statement )
{
    // This is slow. :(
    // We should really do this on storage level!
    QList<Statement> statementsToRemove = listStatements( statement ).allStatements();

    // remove the statements
    Soprano::Error::ErrorCode c = FilterModel::removeAllStatements( statement );

    // remove dangling graphs
    if ( c == Error::ErrorNone ) {
        QSet<QUrl> contexts;
        for ( QList<Statement>::const_iterator it = statementsToRemove.constBegin(); it != statementsToRemove.constEnd(); ++it ) {
            contexts.insert( it->context().uri() );
        }
        for ( QSet<QUrl>::const_iterator it = contexts.constBegin(); it != contexts.constEnd(); ++it ) {
            if ( ( c = removeGraphIfEmpty( *it ) ) != Error::ErrorNone ) {
                return c;
            }
        }
    }

    return Error::ErrorNone;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::removeGraphIfEmpty( const Node& graph )
{
    if ( graph.isEmpty() ) {
        return Error::ErrorNone;
    }

    Soprano::Error::ErrorCode c = Error::ErrorNone;

    // metadata graphs contain type information about themselves. They do not count as "real" content.
    if ( !executeQuery( QString("ask where { graph <%1> { ?s ?p ?o . } . FILTER(?s != <%1>) .}")
                        .arg( QString::fromAscii( graph.uri().toEncoded() ) ),
                        Query::QueryLanguageSparql ).boolValue() ) {

        // do not do anything stupid if the backend fails to handle graph queries
        if ( lastError() ) {
            return Error::convertErrorCode( lastError().code() );
        }

        // remove the graph itself (do not use the direct call which will result in a recursion)
        if ( ( c = parentModel()->removeContext( graph ) ) != Error::ErrorNone ) {
            setError( parentModel()->lastError() );
            return c;
        }

        // remove the graph metadata
        if ( ( c = parentModel()->removeAllStatements( graph, Node(), Node() ) ) != Error::ErrorNone ) {
            setError( parentModel()->lastError() );
            return c;
        }

        // remove a dangling metadata graph
        QueryResultIterator metaIt = executeQuery( QString("select ?mg where { ?mg <%1> <%2> . }")
                                                   .arg( Soprano::Vocabulary::NRL::graphMetadataFor().toString() )
                                                   .arg( QString::fromAscii( graph.uri().toEncoded() ) ),
                                                   Query::QueryLanguageSparql );
        if ( metaIt.next() ) {
            Node g = metaIt.binding( "mg" );
            metaIt.close();
            return removeGraphIfEmpty( g );
        }
    }
    return c;
}


Soprano::Error::ErrorCode Nepomuk::ResourceFilterModel::addStatements( const QList<Soprano::Statement>& statements )
{
    QUrl newContext = ResourceManager::instance()->generateUniqueUri();
    QList<Statement> newStatements;
    QList<Statement>::const_iterator end = statements.constEnd();
    for ( QList<Statement>::const_iterator it = statements.constBegin(); it != end; ++it ) {
        Statement s( *it );
        s.setContext( newContext );
        newStatements.append( s );
    }

    Soprano::Error::ErrorCode r = FilterModel::addStatements( newStatements );
    if ( r == Error::ErrorNone ) {
        r = addStatement( Statement( newContext, Soprano::Vocabulary::RDF::type(), Soprano::Vocabulary::NRL::InstanceBase() ) );
        r = addStatement( Statement( newContext, Soprano::Vocabulary::NAO::created(), LiteralValue( QDateTime::currentDateTime() ) ) );
    }
    return r;
}


Soprano::QueryResultIterator Nepomuk::ResourceFilterModel::instanceQuery( const QString& query, const QDateTime& start, const QDateTime& end )
{
    QString newQuery;
    int pos = query.indexOf( '{' );
    if ( pos < 0 ) {
        kDebug() << "No opening bracket found.";
        return 0;
    }

    // keep the select part
    newQuery = query.left( pos+1 );

    // extract the graph patterns (we need them to be separated by space + dot
    int endPos = query.lastIndexOf( '}' );
    QStringList patterns = query.mid( pos+1, endPos-pos-1 ).simplified().split( " .", QString::SkipEmptyParts );

    // create the new graph patterns
    QStringList graphPatterns;
    int graphIndex = 1;
    for ( int i = 0; i < patterns.count(); ++i ) {
        graphPatterns << QString( "graph ?g%1 { %2 . }" ).arg(graphIndex++).arg( patterns[i].simplified() );
    }

    // create graph pattern for nrl:InstanceBase
    graphIndex = 1;
    for ( int i = 0; i < patterns.count(); ++i ) {
        graphPatterns << QString( "?g%1 a <%2>" ).arg(graphIndex++).arg( Soprano::Vocabulary::NRL::InstanceBase().toString() );
    }

    // create date nao:created graph patterns for matching the creation date in the filters
    if ( start.isValid() || end.isValid() ) {
        graphIndex = 1;
        for ( int i = 0; i < patterns.count(); ++i ) {
            graphPatterns << QString( "?g%1 <%2> ?d%1" ).arg(graphIndex++).arg( Soprano::Vocabulary::NAO::created().toString() );
        }
    }

    // create optional additional filters
    QStringList filters;

    // create start filter
    if ( start.isValid() ) {
        QString filter = "FILTER( ";
        graphIndex = 1;
        Soprano::LiteralValue startLiteral( start );
        for ( int i = 0; i < patterns.count(); ++i ) {
            filter += QString( "?d%1 >= '%2'^^<%3>" ).arg( graphIndex++ ).arg( startLiteral.toString() ).arg( startLiteral.dataTypeUri().toString() );
            if ( i+1 < patterns.count() ) {
                filter += " || ";
            }
        }
        filter += " )";
        filters << filter;
    }

    // create end filter
    if ( end.isValid() ) {
        QString filter = "FILTER( ";
        graphIndex = 1;
        Soprano::LiteralValue endLiteral( end );
        for ( int i = 0; i < patterns.count(); ++i ) {
            filter += QString( "?d%1 <= '%2'^^<%3>" ).arg( graphIndex++ ).arg( endLiteral.toString() ).arg( endLiteral.dataTypeUri().toString() );
            if ( i+1 < patterns.count() ) {
                filter += " || ";
            }
        }
        filter += " )";
        filters << filter;
    }

    newQuery += ' ' + ( graphPatterns + filters ).join( " . " );
    newQuery += " . }";

    return executeQuery( newQuery,  Soprano::Query::QueryLanguageSparql );
}
