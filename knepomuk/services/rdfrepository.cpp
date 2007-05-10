/*
 *
 * $Id: sourceheader 511311 2006-02-19 14:51:05Z trueg $
 *
 * This file is part of the Nepomuk KDE project.
 * Copyright (C) 2006 Sebastian Trueg <trueg@kde.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file "COPYING.LIB" for the exact licensing terms.
 */

#include "rdfrepository.h"
#include <knepomuk/service.h>
#include <knepomuk/message.h>
#include <knepomuk/result.h>
#include "../rdf/rdfdbus.h"

#include <kdebug.h>


Nepomuk::Services::RDFRepository::RDFRepository( Nepomuk::Backbone::Service* s )
    : Backbone::ServiceWrapper( s )
{
}


void Nepomuk::Services::RDFRepository::createRepository( const QString& repid )
{
    Backbone::Message m( service()->url(), "createRepository" );
    m.addArgument( repid );

    setLastResult( service()->methodCall( m ) );
}


QStringList Nepomuk::Services::RDFRepository::listRepositoriyIds( )
{
    Backbone::Message m( service()->url(), "listRepositoriyIds" );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "removeRepository" );
    m.addArgument( repid );

    setLastResult( service()->methodCall( m ) );
}


int Nepomuk::Services::RDFRepository::getRepositorySize( const QString& repid )
{
    Backbone::Message m( service()->url(), "getRepositorySize" );
    m.addArgument( repid );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "contains" );
    m.addArgument( repid );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "addStatement" );
    m.addArgument( repid );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    service()->methodCall( m );
}


void Nepomuk::Services::RDFRepository::addStatements( const QString& graphId,
						      const QList<Soprano::Statement>& statements )
{
    Backbone::Message m( service()->url(), "addStatements" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statements );
    m.addArgument( v );

    setLastResult( service()->methodCall( m ) );
}


void Nepomuk::Services::RDFRepository::removeContext( const QString& graphId,
						      const Soprano::Node& context )
{
    Backbone::Message m( service()->url(), "removeContext" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( context );
    m.addArgument( v );

    setLastResult( service()->methodCall( m ) );
}


int Nepomuk::Services::RDFRepository::removeStatement( const QString& graphId,
						       const Soprano::Statement& statement )
{
    Backbone::Message m( service()->url(), "removeStatement" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "removeStatements" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statements );
    m.addArgument( v );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "removeAllStatements" );
    m.addArgument( graphId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "listStatements" );
    m.addArgument( repositoryId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::constructSparql( const QString& repositoryId, const QString& query )
{
    Backbone::Message m( service()->url(), "constructSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::RDFRepository::selectSparql( const QString& repositoryId, const QString& query )
{
    Backbone::Message m( service()->url(), "selectSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<RDF::QueryResultTable>();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::describeSparql( const QString& repositoryId, const QString& query )
{
    Backbone::Message m( service()->url(), "describeSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::construct( const QString& repositoryId, const QString& query,
									    const QString& querylanguage )
{
    Backbone::Message m( service()->url(), "construct" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::RDFRepository::select( const QString& repositoryId, const QString& query,
									 const QString& querylanguage )
{
    Backbone::Message m( service()->url(), "select" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<RDF::QueryResultTable>();
}


int Nepomuk::Services::RDFRepository::queryListStatements( const QString& repositoryId, const Soprano::Statement& statement,
							   int timeoutMSec )
{
    Backbone::Message m( service()->url(), "queryListStatements" );
    m.addArgument( repositoryId );
    QVariant v;
    v.setValue( statement );
    m.addArgument( v );
    m.addArgument( timeoutMSec );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "queryConstruct" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );
    m.addArgument( timeoutMSec );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "querySelect" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( querylanguage );
    m.addArgument( timeoutMSec );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "queryConstructSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( timeoutMSec );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "querySelectSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( timeoutMSec );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "queryDescribeSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );
    m.addArgument( timeoutMSec );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "askSparql" );
    m.addArgument( repositoryId );
    m.addArgument( query );

    Backbone::Result reply = service()->methodCall( m );
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
    Backbone::Message m( service()->url(), "fetchListStatementsResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::fetchConstructResults( int queryId, int max )
{
    Backbone::Message m( service()->url(), "fetchConstructResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


QList<Soprano::Statement> Nepomuk::Services::RDFRepository::fetchDescribeResults( int queryId, int max )
{
    Backbone::Message m( service()->url(), "fetchDescribeResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QList<Soprano::Statement> >();
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::RDFRepository::fetchSelectResults( int queryId, int max )
{
    Backbone::Message m( service()->url(), "fetchSelectResults" );
    m.addArgument( queryId );
    m.addArgument( max );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<RDF::QueryResultTable>();
}


void Nepomuk::Services::RDFRepository::closeQuery( int listId )
{
    Backbone::Message m( service()->url(), "closeQuery" );
    m.addArgument( listId );

    setLastResult( service()->methodCall( m ) );
}


QStringList Nepomuk::Services::RDFRepository::supportedQueryLanguages()
{
    Backbone::Message m( service()->url(), "supportedQueryLanguages" );
    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QStringList>();
}


int Nepomuk::Services::RDFRepository::supportsQueryLanguage( const QString& lang )
{
    Backbone::Message m( service()->url(), "supportsQueryLanguage" );
    m.addArgument( lang );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() )
        return reply.value().value<int>();
    else
        return 0;
}


QStringList Nepomuk::Services::RDFRepository::supportedSerializations()
{
    Backbone::Message m( service()->url(), "supportedSerializations" );
    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    return reply.value().value<QStringList>();
}


int Nepomuk::Services::RDFRepository::supportsSerialization( const QString& serializationMimeType )
{
    Backbone::Message m( service()->url(), "supportsSerialization" );
    m.addArgument( serializationMimeType );

    Backbone::Result reply = service()->methodCall( m );
    setLastResult( reply );
    if( !reply.status() )
        return reply.value().value<int>();
    else
        return 0;
}


void Nepomuk::Services::RDFRepository::addGraph( const QString& repositoryId, const QString& graph,
						 const QString& formatMimetype, const Soprano::Node& context )
{
    Backbone::Message m( service()->url(), "addGraph" );
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
    Backbone::Message m( service()->url(), "removeGraph" );
    m.addArgument( repositoryId );
    m.addArgument( graph );
    m.addArgument( formatMimetype );
    QVariant v;
    v.setValue( context );
    m.addArgument( v );

    setLastResult( service()->methodCall( m ) );
}
