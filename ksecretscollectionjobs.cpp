/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
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

#include "ksecretscollectionjobs.h"
#include "ksecretscollectionjobs_p.h"
#include "ksecretsservicecollection_p.h"
#include "service_interface.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <kdebug.h>

using namespace KSecretsService;

CollectionJobPrivate::CollectionJobPrivate() :
    collection( 0 ), error( CollectionJob::NoError )
{
}

CollectionJob::CollectionJob(Collection *collection, QObject* parent) : 
            KCompositeJob( parent ), 
            d( new CollectionJobPrivate() )
{
    d->collection = collection;
}

CollectionJob::CollectionError CollectionJob::error() const 
{ 
    return d->error; 
}

Collection *CollectionJob::collection() const 
{ 
    return d->collection; 
}

void CollectionJob::finishedWithError( CollectionError err, const QString &errTxt )
{
    // FIXME: check that this will also abort the parent job
    setError( err );
    setErrorText( errTxt );
    emitResult();
}

void CollectionJob::finishedOk()
{
    setError( NoError );
    emitResult();
}

bool CollectionJob::addSubjob( KJob* theJob )
{
    return KCompositeJob::addSubjob( theJob );
}

void CollectionJob::startFindCollection()
{
    if ( !d->collection->d->isValid() ) {
        FindCollectionJob *findJob = new FindCollectionJob( d->collection, this );
        if ( addSubjob( findJob ) ) {
            findJob->start();
        }
        else {
            kDebug() << "FindCollectionJob failed to start";
            setError( InternalError );
            emitResult();
        }
    }
    else {
        // collection was already found or created, just trigger this 
        onFindCollectionFinished();
    }
}

void CollectionJob::slotResult(KJob* job)
{
    KCompositeJob::slotResult(job);
    // TODO: check if the subjob result here
    FindCollectionJob *findJob = dynamic_cast< FindCollectionJob* >( job );
    Q_ASSERT( findJob != 0 );
    // FIXME: see if this call should transport error codes
    onFindCollectionFinished();
}

void CollectionJob::onFindCollectionFinished()
{
    // nothing to do in this base implementation
}


FindCollectionJob::FindCollectionJob(   Collection *collection, 
                                        QObject *parent ) : 
            CollectionJob( collection, parent ),
            d( new FindCollectionJobPrivate( this, collection->d ) )
{
    d->collectionName = collection->d->collectioName;
    d->findCollectionOptions = collection->d->findOptions;
}

void FindCollectionJob::start() 
{
    // meanwhile another findJob instance would have already connected our collection object
    if ( ! collection()->d->isValid() ) {
        if ( collection()->d->findOptions == Collection::CreateCollection ) {
            d->startCreateCollection();
        }
        else 
            if ( collection()->d->findOptions == Collection::OpenOnly ) {
                d->startOpenCollection();
            }
            else {
                Q_ASSERT( 0 ); // "Unknown findOtions" );
            }
    }
    else {
        setError( 0 );
        emitResult();
    }
}

FindCollectionJobPrivate::FindCollectionJobPrivate(FindCollectionJob *fcj, CollectionPrivate *cp ) :
        findJob( fcj), collectionPrivate( cp )
{
}

void FindCollectionJobPrivate::createFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > createReply = *watcher;
    Q_ASSERT( createReply.isFinished() ); // "on ne sait jamais"
    if ( watcher->isError() ) {
        kDebug() << "creating collection '" << collectionName << "' failed";
        kDebug() << "DBusError.type = " << createReply.error().type();
        kDebug() << "DBusError.name = " << createReply.error().name();
        kDebug() << "DBusError.message = " << createReply.error().message();
        findJob->finishedWithError( CollectionJob::CreateError, createReply.error().message() );
    }
    else {
        QDBusObjectPath collPath = createReply.argumentAt<0>();
        QDBusObjectPath promptPath = createReply.argumentAt<1>();
        
        if ( collPath.path().compare("/") == 0 ) {
            // we need prompting
            Q_ASSERT( promptPath.path().compare("/") ); // we should have a prompt path here
            PromptJob *promptJob = new PromptJob( promptPath, this );
            if ( findJob->addSubjob( promptJob ) ) {
                connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(createPromptFinished(KJob*)) );
                promptJob->start();
            }
            else {
                promptJob->deleteLater();
                kDebug() << "cannot add prompt subjob!";
                findJob->finishedWithError( CollectionJob::InternalError, "Cannot add prompt job" );
            }
        }
        else {
            findJob->d->collectionPrivate->setDBusPath( collPath );
            findJob->finishedOk();
        }
    }
}

void FindCollectionJobPrivate::createPromptFinished( KJob* job )
{
    PromptJob *promptJob = dynamic_cast< PromptJob* >( job );
    if ( promptJob->error() == 0 ) {
        QDBusVariant promptResult = promptJob->operationResult();
        QDBusObjectPath collPath = promptResult.variant().value< QDBusObjectPath >();
        collectionPrivate->setDBusPath( collPath );
        findJob->finishedOk();
    }
    else {
        findJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, "The operation was cancelled by the user" );
    }
}


void FindCollectionJobPrivate::startCreateCollection()
{
    OpenSessionJob *openSessionJob = DBusSession::service();
    findJob->addSubjob( openSessionJob );
    connect( openSessionJob, SIGNAL(finished(KJob*)), this, SLOT(openSessionFinished(KJOb*)) );
    openSessionJob->start();
}

void FindCollectionJobPrivate::openSessionFinished(KJob* theJob)
{
    OpenSessionJob *openSessionJob = dynamic_cast< OpenSessionJob * >( theJob );
    QVariantMap creationProperties;
    QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > createReply = openSessionJob->serviceInterface()->CreateCollection(
        creationProperties, collectionName );
    QDBusPendingCallWatcher *createReplyWatch = new QDBusPendingCallWatcher( createReply, this );
    connect( createReplyWatch, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createFinished(QDBusPendingCallWatcher*)) );
}

void FindCollectionJobPrivate::startOpenCollection()
{
    // TODO: implement this
}


DeleteCollectionJob::DeleteCollectionJob( Collection* collection, QObject* parent ) :
        CollectionJob( collection, parent),
        d( new DeleteCollectionJobPrivate(this) )
{
}
    
void DeleteCollectionJob::start() 
{
    startFindCollection();
    // this will trigger onFindCollectionFinished
}

void DeleteCollectionJob::onFindCollectionFinished() 
{
    // now performe the real delete operation on the backend
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( d->deleteReply, this );
    connect( watcher, SIGNAL( finished(QDBusPendingCallWatcher*) ), d.data(), SLOT( callFinished(QDBusPendingCallWatcher*) ) );
    connect( d.data(), SIGNAL( deleteIsDone( CollectionError, const QString & ) ), d.data(), SLOT( deleteIsDone( CollectionError, const QString & ) ) );
}

void KSecretsService::DeleteCollectionJob::deleteIsDone(CollectionJob::CollectionError err, const QString& errMsg )
{
    finishedWithError( err, errMsg );
}

DeleteCollectionJobPrivate::DeleteCollectionJobPrivate( QObject* parent ) : QObject( parent )
{
}

void DeleteCollectionJobPrivate::callFinished( QDBusPendingCallWatcher* /* watcher */ ) 
{
    Q_ASSERT( deleteReply.isFinished() );

    CollectionJob::CollectionError err = CollectionJob::NoError;
    QString msg;
    
    if ( deleteReply.isError() ) {
        err = CollectionJob::DeleteError;
        const QDBusError &dbusErr = deleteReply.error();
        msg = QString("d-bus error %1 (%2)").arg( QDBusError::errorString( dbusErr.type() ) ).arg( dbusErr.message() );
    }

    kDebug() << "callFinished with err=" << (int)err << " and msg='" << msg << "'";
    emit deleteIsDone( err, msg );
}


Collection::SearchItemsJob::SearchItemsJob( Collection *collection,
                                            QObject *parent ) :
    CollectionJob( collection, parent ) 
{
}


Collection::SearchSecretsJob::SearchSecretsJob( Collection* collection, QObject* parent ) : 
    CollectionJob( collection, parent )
{
}

Collection::CreateItemJob::CreateItemJob( Collection *collection,
                                          QObject *parent ) :
    CollectionJob( collection, parent )
{
}

Collection::ReadItemsJob::ReadItemsJob( Collection *collection,
                                        QObject *parent ) :
    CollectionJob( collection, parent )
{
}
    
QList< Secret > Collection::SearchSecretsJob::secrets() const
{
    // TODO: implement this
    return QList< Secret >();
}

PromptJob::PromptJob( const QDBusObjectPath &path, QObject *parent ) : KJob( parent )
{
}

void PromptJob::start()
{
    // TODO: implement this
}


#include "ksecretscollectionjobs.moc"
#include "ksecretscollectionjobs_p.moc"
