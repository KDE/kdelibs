/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006-2007 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "rdfrepository.h"
#include "service.h"
#include "message.h"
#include "result.h"
#include "rdf/rdfdbus.h"

#include <kdebug.h>


Nepomuk::Services::RDFRepository::RDFRepository( Nepomuk::Middleware::Service* s )
    : Middleware::ServiceWrapper( s )
{
}


void Nepomuk::Services::RDFRepository::createRepository( const QString& repid )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "createRepository" );
    m.addArgument( repid );

    setLastResult( service()->methodCall( m ) );
}


QStringList Nepomuk::Services::RDFRepository::listRepositoryIds( )
{
    if ( !service() ) {
        return QStringList();
    }

    Middleware::Message m( service()->url(), "listRepositoryIds" );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() ) {
        return reply.value().toStringList();
    }
    else {
        return QStringList();
    }
}


void Nepomuk::Services::RDFRepository::removeRepository( const QString& repid )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "removeRepository" );
    m.addArgument( repid );

    setLastResult( service()->methodCall( m ) );
}


int Nepomuk::Services::RDFRepository::getRepositorySize( const QString& repid )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "getRepositorySize" );
    m.addArgument( repid );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::contains( const QString& repid, const Soprano::Statement& statement )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "contains" );
    m.addArgument( repid );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


void Nepomuk::Services::RDFRepository::addStatement( const QString& repid,
						     const Soprano::Statement& statement )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "addStatement" );
    m.addArgument( repid );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    service()->methodCall( m );
}


void Nepomuk::Services::RDFRepository::addStatements( const QString& graphId,
						      const QList<Soprano::Statement>& statements )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "addStatements" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statements );
    m.addArgument( v );

    setLastResult( service()->methodCall( m ) );
}


void Nepomuk::Services::RDFRepository::removeContext( const QString& graphId,
						      const Soprano::Node& context )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "removeContext" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( context );
    m.addArgument( v );

    setLastResult( service()->methodCall( m ) );
}


int Nepomuk::Services::RDFRepository::removeStatement( const QString& graphId,
						       const Soprano::Statement& statement )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "removeStatement" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::removeStatements( const QString& graphId,
							const QList<Soprano::Statement>& statements )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "removeStatements" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statements );
    m.addArgument( v );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::removeAllStatements( const QString& graphId,
							   const Soprano::Statement& statement )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "removeAllStatements" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


QList<Soprano::Statement>
Nepomuk::Services::RDFRepository::listStatements( const QString& repositoryId, const Soprano::Statement& statement )
{
    if ( !service() ) {
        return QList<Soprano::Statement>();
    }

    Middleware::Message m( service()->url(), "listStatements" );
    m.addArgument( repositoryId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::constructSparql( const QString& repositoryId, const QString& query )
{
    if ( !service() ) {
        return QList<Soprano::Statement>();
    }

    Middleware::Message m( service()->url(), "constructSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::RDFRepository::selectSparql( const QString& repositoryId, const QString& query )
{
    if ( !service() ) {
        return Nepomuk::RDF::QueryResultTable();
    }

    Middleware::Message m( service()->url(), "selectSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<RDF::QueryResultTable>();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::describeSparql( const QString& repositoryId, const QString& query )
{
    if ( !service() ) {
        return QList<Soprano::Statement>();
    }

    Middleware::Message m( service()->url(), "describeSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::construct( const QString& repositoryId, const QString& query,
									    const QString& querylanguage )
{
    if ( !service() ) {
        return QList<Soprano::Statement>();
    }

    Middleware::Message m( service()->url(), "construct" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::RDFRepository::select( const QString& repositoryId, const QString& query,
									 const QString& querylanguage )
{
    if ( !service() ) {
        return Nepomuk::RDF::QueryResultTable();
    }

    Middleware::Message m( service()->url(), "select" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<RDF::QueryResultTable>();
}


int Nepomuk::Services::RDFRepository::queryListStatements( const QString& repositoryId, const Soprano::Statement& statement,
							   int timeoutMSec )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "queryListStatements" );
    m.addArgument( repositoryId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );
    m.addArgument( timeoutMSec );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::queryConstruct( const QString& repositoryId, const QString& query,
						      const QString& querylanguage, int timeoutMSec )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "queryConstruct" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );
    m.addArgument( timeoutMSec );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::querySelect( const QString& repositoryId, const QString& query,
						   const QString& querylanguage, int timeoutMSec )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "querySelect" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );
    m.addArgument( timeoutMSec );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::queryConstructSparql( const QString& repositoryId, const QString& query,
							    int timeoutMSec )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "queryConstructSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( timeoutMSec );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::querySelectSparql( const QString& repositoryId, const QString& query,
							 int timeoutMSec )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "querySelectSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( timeoutMSec );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}

int Nepomuk::Services::RDFRepository::queryDescribeSparql( const QString& repositoryId, const QString& query,
							   int timeoutMSec )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "queryDescribeSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( timeoutMSec );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


int Nepomuk::Services::RDFRepository::askSparql( const QString& repositoryId, const QString& query )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "askSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() && reply.value().type() == QVariant::Int ) {
        return reply.value().toInt();
    }
    else {
        return 0;
    }
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::fetchListStatementsResults( int queryId, int max )
{
    if ( !service() ) {
        return QList<Soprano::Statement>();
    }

    Middleware::Message m( service()->url(), "fetchListStatementsResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::fetchConstructResults( int queryId, int max )
{
    if ( !service() ) {
        return QList<Soprano::Statement>();
    }

    Middleware::Message m( service()->url(), "fetchConstructResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::fetchDescribeResults( int queryId, int max )
{
    if ( !service() ) {
        return QList<Soprano::Statement>();
    }

    Middleware::Message m( service()->url(), "fetchDescribeResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::RDFRepository::fetchSelectResults( int queryId, int max )
{
    if ( !service() ) {
        return Nepomuk::RDF::QueryResultTable();
    }

    Middleware::Message m( service()->url(), "fetchSelectResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<RDF::QueryResultTable>();
}


void Nepomuk::Services::RDFRepository::closeQuery( int listId )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "closeQuery" );
    m.addArgument( listId );

    setLastResult( service()->methodCall( m ) );
}


QStringList Nepomuk::Services::RDFRepository::supportedQueryLanguages()
{
    if ( !service() ) {
        return QStringList();
    }

    Middleware::Message m( service()->url(), "supportedQueryLanguages" );
    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QStringList>();
}


int Nepomuk::Services::RDFRepository::supportsQueryLanguage( const QString& lang )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "supportsQueryLanguage" );
    m.addArgument( lang );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() )
        return reply.value().value<int>();
    else
        return 0;
}


QStringList Nepomuk::Services::RDFRepository::supportedSerializations()
{
    if ( !service() ) {
        return QStringList();
    }

    Middleware::Message m( service()->url(), "supportedSerializations" );
    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QStringList>();
}


int Nepomuk::Services::RDFRepository::supportsSerialization( const QString& serializationMimeType )
{
    if ( !service() ) {
        return 0;
    }

    Middleware::Message m( service()->url(), "supportsSerialization" );
    m.addArgument( serializationMimeType );

    Middleware::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() )
        return reply.value().value<int>();
    else
        return 0;
}


void Nepomuk::Services::RDFRepository::addGraph( const QString& repositoryId, const QString& graph,
						 const QString& formatMimetype, const Soprano::Node& context )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "addGraph" );
    m.addArgument( repositoryId );
    m.addArgument( graph );
    m.addArgument( formatMimetype );
    QVariant v;
    v.setValue( context );
    m.addArgument( v );

    setLastResult( service()->methodCall( m ) );
}


void Nepomuk::Services::RDFRepository::removeGraph( const QString& repositoryId, const QString& graph,
						    const QString& formatMimetype, const Soprano::Node& context )
{
    if ( !service() ) {
        return;
    }

    Middleware::Message m( service()->url(), "removeGraph" );
    m.addArgument( repositoryId );
    m.addArgument( graph );
    m.addArgument( formatMimetype );
    QVariant v;
    v.setValue( context );
    m.addArgument( v );

    setLastResult( service()->methodCall( m ) );
}
