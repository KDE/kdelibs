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

CollectionJob::CollectionJob(Collection *collection, QObject* parent, bool shouldTriggerFind) : 
            KCompositeJob( parent ), 
            collection( collection ),
            _error( UndefinedError ) 
{
}

void CollectionJob::startFindCollection()
{
    if ( !collection->d->isValid() ) {
        FindCollectionJob *findJob = new FindCollectionJob( collection, collection->d->collectioName, collection->d->findOptions, this );
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
                                        const QString& collName,
                                        Collection::FindCollectionOptions options,
                                        QObject *parent ) : 
            CollectionJob( collection, parent, false ), // give false here to avoid infinite loops
            d( new FindCollectionJobPrivate() )
{
    d->collectionName = collName;
    d->findCollectionOptions = options;
}

void FindCollectionJob::start() 
{
    // meanwhile another findJob instance would have already connected our collection object
    if ( ! collection->d->isValid() ) {
        if ( collection->d->findOptions == Collection::CreateCollection ) {
            OrgFreedesktopSecretServiceInterface *service = DBusSession::service();
            QVariantMap creationProperties;
            QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > createReply = service->CreateCollection(
                creationProperties, collection->d->collectioName );
            QDBusPendingCallWatcher *createReplyWatch = new QDBusPendingCallWatcher( createReply, this );
            connect( createReplyWatch, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createFinished(QDBusPendingCallWatcher*)) );
        }
        else 
            if ( collection->d->findOptions == Collection::CreateCollection ) {
                // TODO: implement this
            }
            else {
                Q_ASSERT( 0 ); // "Unknown findOptions" );
            }
    }
    else {
        setError( 0 );
        emitResult();
    }
}

FindCollectionJobPrivate::FindCollectionJobPrivate() :
        findCollectionOptions( Collection::OpenOnly )
{
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
    _error = err;
    setError( (int)err ); 
    setErrorText( errMsg );
    emitResult();
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



#include "ksecretscollectionjobs.moc"
#include "ksecretscollectionjobs_p.moc"
