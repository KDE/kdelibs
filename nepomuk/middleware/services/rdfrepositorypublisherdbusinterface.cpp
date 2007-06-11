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

#include "rdfrepositorypublisherdbusinterface.h"
#include "rdfrepositorypublisher.h"

#include "error.h"

Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::RDFRepositoryPublisherInterface( Nepomuk::Middleware::ServicePublisher* p )
    : Middleware::DBus::ServicePublisherInterface( p )
{
}


void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::createRepository( const QString& repid, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    static_cast<RDFRepositoryPublisher*>( servicePublisher() )->createRepository( repid );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}

QStringList Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::listRepositoryIds( const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QStringList l = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->listRepositoryIds();
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return l;
}


void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::removeRepository( const QString& repositoryId, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    static_cast<RDFRepositoryPublisher*>( servicePublisher() )->removeRepository( repositoryId );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::getRepositorySize( const QString& repositoryId, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->getRepositorySize( repositoryId );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::contains( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->contains( repositoryId, statement );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::addStatement( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    static_cast<RDFRepositoryPublisher*>( servicePublisher() )->addStatement( repositoryId, statement );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}


void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::addStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    static_cast<RDFRepositoryPublisher*>( servicePublisher() )->addStatements( repositoryId, statements );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}


void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::removeContext( const QString& repositoryId, const Soprano::Node& context, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    static_cast<RDFRepositoryPublisher*>( servicePublisher() )->removeContext( repositoryId, context );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::removeStatement( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->removeStatement( repositoryId, statement );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::removeStatements( const QString& repositoryId, const QList<Soprano::Statement>& statements, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->removeStatements( repositoryId, statements );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::removeAllStatements( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->removeAllStatements( repositoryId, statement );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QList<Soprano::Statement> Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::listStatements( const QString& repositoryId, const Soprano::Statement& statement, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QList<Soprano::Statement> r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->listStatements( repositoryId, statement );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QList<Soprano::Statement> Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::constructSparql( const QString& repositoryId, const QString& query, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QList<Soprano::Statement> r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->constructSparql( repositoryId, query );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::selectSparql( const QString& repositoryId, const QString& query, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    Nepomuk::RDF::QueryResultTable r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->selectSparql( repositoryId, query );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QList<Soprano::Statement> Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::describeSparql( const QString& repositoryId, const QString& query, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QList<Soprano::Statement> r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->describeSparql( repositoryId, query );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QList<Soprano::Statement> Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::construct( const QString& repositoryId, const QString& query,
												    const QString& querylanguage, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QList<Soprano::Statement> r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->construct( repositoryId, query, querylanguage );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::select( const QString& repositoryId, const QString& query,
												 const QString& querylangauge, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    Nepomuk::RDF::QueryResultTable r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->select( repositoryId, query, querylangauge );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::queryListStatements( const QString& repositoryId, const Soprano::Statement& statement,
										   int timeoutMSec, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->queryListStatements( repositoryId, statement, timeoutMSec );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::queryConstruct( const QString& repositoryId, const QString& query,
									      const QString& querylanguage, int timeoutMSec, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->queryConstruct( repositoryId, query, querylanguage, timeoutMSec );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::querySelect( const QString& repositoryId, const QString& query,
									   const QString& querylanguage, int timeoutMSec, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->querySelect( repositoryId, query, querylanguage, timeoutMSec );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::queryConstructSparql( const QString& repositoryId, const QString& query,
										    int timeoutMSec, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->queryConstructSparql( repositoryId, query, timeoutMSec );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::querySelectSparql( const QString& repositoryId, const QString& query,
										 int timeoutMSec, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->querySelectSparql( repositoryId, query, timeoutMSec );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::queryDescribeSparql( const QString& repositoryId, const QString& query,
										   int timeoutMSec, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->queryDescribeSparql( repositoryId, query, timeoutMSec );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::askSparql( const QString& repositoryId, const QString& query, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->askSparql( repositoryId, query );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QList<Soprano::Statement> Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::fetchListStatementsResults( int queryId, int max, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QList<Soprano::Statement> r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->fetchListStatementsResults( queryId, max );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QList<Soprano::Statement> Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::fetchConstructResults( int queryId, int max, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QList<Soprano::Statement> r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->fetchConstructResults( queryId, max );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QList<Soprano::Statement> Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::fetchDescribeResults( int queryId, int max, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QList<Soprano::Statement> r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->fetchDescribeResults( queryId, max );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


Nepomuk::RDF::QueryResultTable Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::fetchSelectResults( int queryId, int max, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    Nepomuk::RDF::QueryResultTable r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->fetchSelectResults( queryId, max );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::closeQuery( int queryId, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    return static_cast<RDFRepositoryPublisher*>( servicePublisher() )->closeQuery( queryId );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}


QStringList Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::supportedQueryLanguages( const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QStringList r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->supportedQueryLanguages();
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::supportsQueryLanguage( const QString& lang, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->supportsQueryLanguage( lang );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


QStringList Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::supportedSerializations( const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    QStringList r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->supportedSerializations();
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


int Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::supportsSerialization( const QString& serializationMimeType, const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    int r = static_cast<RDFRepositoryPublisher*>( servicePublisher() )->supportsSerialization( serializationMimeType );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
    return r;
}


void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::addGraph( const QString& repositoryId, const QString& graph,
									 const QString& formatMimetype, const Soprano::Node& context,
									 const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    static_cast<RDFRepositoryPublisher*>( servicePublisher() )->addGraph( repositoryId, graph, formatMimetype, context );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}



void Nepomuk::Services::DBus::RDFRepositoryPublisherInterface::removeGraph( const QString& repositoryId, const QString& graph,
									    const QString& formatMimetype, const Soprano::Node& context,
									    const QDBusMessage& m )
{
    servicePublisher()->setError( Middleware::Error::NoError );
    static_cast<RDFRepositoryPublisher*>( servicePublisher() )->removeGraph( repositoryId, graph, formatMimetype, context );
    if( !servicePublisher()->success() )
        QDBusConnection::sessionBus().send( m.createErrorReply( servicePublisher()->errorName(), servicePublisher()->errorMessage() ) );
}


#include "rdfrepositorypublisherdbusinterface.moc"
