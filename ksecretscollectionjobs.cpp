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
#include "ksecretsserviceitem_p.h"
#include "service_interface.h"
#include "collection_interface.h"
#include "item_interface.h"
#include "../daemon/frontend/secret/adaptors/secretstruct.h"
#include "ksecretsservicesecret_p.h"
#include "promptjob.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <kdebug.h>
#include <prompt_interface.h>
#include <kapplication.h>
#include <QWidget>

using namespace KSecretsService;

CollectionJobPrivate::CollectionJobPrivate() :
    collection( 0 )
{
}

CollectionJob::CollectionJob(Collection *collection, QObject* parent) : 
            KCompositeJob( parent ), 
            d( new CollectionJobPrivate() )
{
    d->collection = collection;
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
    if ( job->error() == 0 ) {
        FindCollectionJob *findJob = dynamic_cast< FindCollectionJob* >( job );
        if ( findJob != 0 ) {
            onFindCollectionFinished();
        }
    }
}

void CollectionJob::onFindCollectionFinished()
{
    // nothing to do in this base implementation
}


FindCollectionJob::FindCollectionJob(   Collection *collection, 
                                        QObject *parent ) : 
            CollectionJob( collection, parent ),
            d( new FindCollectionJobPrivate( this, collection->d.data() ) )
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
            PromptJob *promptJob = new PromptJob( promptPath, collectionPrivate->promptParentId(), this );
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
    watcher->deleteLater();
}

void FindCollectionJobPrivate::createPromptFinished( KJob* job )
{
    PromptJob *promptJob = dynamic_cast< PromptJob* >( job );
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            QDBusVariant promptResult = promptJob->result();
            QDBusObjectPath collPath = promptResult.variant().value< QDBusObjectPath >();
            collectionPrivate->setDBusPath( collPath );
            findJob->finishedOk();
        }
        else {
            findJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, "The operation was cancelled by the user" );
        }
    }
    else {
        findJob->finishedWithError( CollectionJob::InternalError, "Error encountered when trying to prompt the user" );
    }
}


void FindCollectionJobPrivate::startCreateCollection()
{
    OpenSessionJob *openSessionJob = DBusSession::openSession();
    if ( findJob->addSubjob( openSessionJob ) ) {
        connect( openSessionJob, SIGNAL(finished(KJob*)), this, SLOT(openSessionFinished(KJob*)) );
        openSessionJob->start();
    }
    else {
        kDebug() << "Cannot OpenSessionJob subjob";
        findJob->finishedWithError( CollectionJob::InternalError, "Cannot open session" );
    }
}

void FindCollectionJobPrivate::openSessionFinished(KJob* theJob)
{
    if ( !theJob->error() ) {
        OpenSessionJob *openSessionJob = dynamic_cast< OpenSessionJob * >( theJob );
        QVariantMap creationProperties;
        creationProperties.insert("org.freedesktop.Secret.Collection.Label", collectionName);
        QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > createReply = openSessionJob->serviceInterface()->CreateCollection(
            creationProperties, collectionName );
        QDBusPendingCallWatcher *createReplyWatch = new QDBusPendingCallWatcher( createReply, this );
        connect( createReplyWatch, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createFinished(QDBusPendingCallWatcher*)) );
    }
}

void FindCollectionJobPrivate::startOpenCollection()
{
    // TODO: implement this
}


DeleteCollectionJob::DeleteCollectionJob( Collection* collection, QObject* parent ) :
        CollectionJob( collection, parent),
        d( new DeleteCollectionJobPrivate( collection->d.data(), this ) )
{
}
    
void DeleteCollectionJob::start() 
{
    // ensure we have the connection to the daemon and we have a valid collection
    // this will trigger onFindCollectionFinished
    startFindCollection();
}

void DeleteCollectionJob::onFindCollectionFinished() 
{
    connect( d.data(), SIGNAL( deleteIsDone( CollectionJob::CollectionError, const QString & ) ), this, SLOT( deleteIsDone( CollectionJob::CollectionError, const QString & ) ) );
    // now performe the real delete operation on the backend
    d->startDelete();
}

void KSecretsService::DeleteCollectionJob::deleteIsDone(CollectionError err, const QString& errMsg )
{
    finishedWithError( err, errMsg );
    d->cp->collectionStatus = Collection::Deleted;
}

DeleteCollectionJobPrivate::DeleteCollectionJobPrivate( CollectionPrivate* collp, QObject* parent ) : 
        QObject( parent ),
        cp( collp )
{
}

void DeleteCollectionJobPrivate::startDelete() 
{
    QDBusPendingReply<QDBusObjectPath> deleteReply = cp->collectionIf->Delete();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( deleteReply, this );
    connect( watcher, SIGNAL( finished(QDBusPendingCallWatcher*) ), this, SLOT( callFinished(QDBusPendingCallWatcher*) ) );
}

void DeleteCollectionJobPrivate::callFinished( QDBusPendingCallWatcher*  watcher ) 
{
    Q_ASSERT( watcher->isFinished() );

    QDBusPendingReply< QDBusObjectPath > deleteReply = *watcher;
    CollectionJob::CollectionError err = CollectionJob::NoError;
    QString msg;
    
    if ( deleteReply.isError() ) {
        err = CollectionJob::DeleteError;
        const QDBusError &dbusErr = deleteReply.error();
        msg = QString("d-bus error %1 (%2)").arg( QDBusError::errorString( dbusErr.type() ) ).arg( dbusErr.message() );
    }

    kDebug() << "callFinished with err=" << (int)err << " and msg='" << msg << "'";
    emit deleteIsDone( err, msg );
    watcher->deleteLater();
}


RenameCollectionJob::RenameCollectionJob( Collection *coll, const QString &newName, QObject *parent ) : 
            CollectionJob( coll, parent ),
            d( new RenameCollectionJobPrivate( coll->d.data(), this ) )
{
    d->newName = newName;
}

void RenameCollectionJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished if collection exists
}

void RenameCollectionJob::onFindCollectionFinished()
{
    connect( d.data(), SIGNAL( renameIsDone( CollectionJob::CollectionError, const QString& ) ), this, SLOT( renameIsDone( CollectionJob::CollectionError, const QString& ) ) );
    d->startRename();
}

void RenameCollectionJob::renameIsDone( CollectionJob::CollectionError err, const QString& msg)
{
    finishedWithError( err, msg );
    // FIXME: should we change the status of newly created collections here? consider the opened status, for example.
}

RenameCollectionJobPrivate::RenameCollectionJobPrivate( CollectionPrivate *collPrivate, QObject *parent ) :
            QObject( parent ),
            collectionPrivate( collPrivate )
{
}

void RenameCollectionJobPrivate::startRename()
{
    if ( collectionPrivate->collectionIf->setProperty( "Label", QVariant( newName ) ) ) {
        emit renameIsDone( CollectionJob::NoError, "" );
    }
    else {
        emit renameIsDone( CollectionJob::RenameError, QString( "Cannot rename collection to %1" ).arg( newName ) );
    }
}

SearchItemsJob::SearchItemsJob( Collection *collection, 
                                const QStringStringMap &attributes,
                                QObject *parent ) :
    CollectionJob( collection, parent ),
    d( new SearchItemsJobPrivate( collection->d.data(), this ) )
{
    d->attributes = attributes;
}

void SearchItemsJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

QList< SecretItem > SearchItemsJob::items() const
{
    QList< SecretItem > items;
    foreach( SecretItemPrivate ip, d->items ) {
        items.append( SecretItem( new SecretItemPrivate( ip ) ) );
    }
    return items;
}

void SearchItemsJob::onFindCollectionFinished()
{
    d->startSearchItems();
}

SearchItemsJobPrivate::SearchItemsJobPrivate( CollectionPrivate* cp, SearchItemsJob *job ) :
    QObject( job ),
    collectionPrivate( cp ),
    searchItemJob( job )
{
}

void SearchItemsJobPrivate::startSearchItems()
{
    QDBusPendingReply< QList< QDBusObjectPath > > reply = collectionPrivate->collectionIf->SearchItems( attributes );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(searchFinished(QDBusPendingCallWatcher*) ) );
}

void SearchItemsJobPrivate::searchFinished(QDBusPendingCallWatcher* watcher)
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply< QList< QDBusObjectPath > > reply = *watcher;
    if ( !reply.isError() ) {
        QList< QDBusObjectPath > itemList = reply.argumentAt<0>();
        foreach( QDBusObjectPath itemPath, itemList ) {
            items.append( SecretItemPrivate( itemPath ) );
        }
        searchItemJob->finishedOk();
    }
    else {
        kDebug() << "ERROR searching items";
        searchItemJob->finishedWithError( CollectionJob::InternalError, "ERROR searching items");
    }
    watcher->deleteLater();
}


SearchSecretsJob::SearchSecretsJob( Collection* collection, const QStringStringMap &attributes, QObject* parent ) : 
    CollectionJob( collection, parent ),
    d( new SearchSecretsJobPrivate( collection->d.data(), attributes ) )
{
}

QList< Secret > SearchSecretsJob::secrets() const
{
    QList< Secret > result;
    foreach( QSharedPointer< SecretPrivate > sp, d->secretsList ) {
        result.append( Secret( sp ) );
    }
    return result;
}

void SearchSecretsJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

void SearchSecretsJob::onFindCollectionFinished()
{
    connect( d.data(), SIGNAL(searchIsDone( CollectionJob::CollectionError, const QString& )), this, SLOT(searchIsDone( CollectionJob::CollectionError, const QString&)) );
    d->startSearchSecrets();
}

void SearchSecretsJob::searchIsDone( CollectionJob::CollectionError err, const QString& msg)
{
    finishedWithError( err, msg );
}

SearchSecretsJobPrivate::SearchSecretsJobPrivate( CollectionPrivate *cp, const QStringStringMap &attrs, QObject *parent ) :
    QObject( parent ),
    collectionPrivate( cp ),
    attributes( attrs )
{
}

void SearchSecretsJobPrivate::startSearchSecrets()
{
    QDBusPendingReply<QList<QDBusObjectPath> > searchReply = collectionPrivate->collectionIf->SearchItems( attributes );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( searchReply, this );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(searchSecretsReply(QDBusPendingCallWatcher*)));
}

void SearchSecretsJobPrivate::searchSecretsReply( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT( watcher );
    QDBusPendingReply<QList<QDBusObjectPath> > searchReply = *watcher;
    if ( !searchReply.isError() ) {
        QList< QDBusObjectPath > pathList = searchReply.value();
        kDebug() << "FOUND " << pathList.count() << " secrets";
        if ( pathList.count() >0 ) {
            QDBusPendingReply<ObjectPathSecretMap> getReply = DBusSession::serviceIf()->GetSecrets( pathList, DBusSession::sessionPath() );
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( getReply );
            connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(getSecretsReply(QDBusPendingCallWatcher*)) );
        }
        else {
           emit searchIsDone( CollectionJob::NoError, "" ); 
        }
    }
    else {
        kDebug() << "ERROR searching items";
        emit searchIsDone( CollectionJob::InternalError, "ERROR searching items" );
    }
    watcher->deleteLater();
}

void SearchSecretsJobPrivate::getSecretsReply(QDBusPendingCallWatcher* watcher)
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply<ObjectPathSecretMap> getReply = *watcher;
    if ( !getReply.isError() ) {
        foreach (SecretStruct secret, getReply.value()) {
            SecretPrivate *sp =0;
            if ( SecretPrivate::fromSecretStrut( secret, sp ) ) {
                secretsList.append( QSharedPointer<SecretPrivate>( sp ) );
            }
            else {
                kDebug() << "ERROR decrypting the secret";
                emit searchIsDone( CollectionJob::InternalError, "ERROR decrypting the secret" );
            }
        }
        emit searchIsDone( CollectionJob::NoError, "" );
    }
    else {
        kDebug() << "ERROR trying to retrieve the secrets";
        emit searchIsDone( CollectionJob::InternalError, "ERROR trying to retrieve the secrets" );
    }
    watcher->deleteLater();
}


CreateItemJob::CreateItemJob( Collection *collection,
                              const QString& label,
                              const QMap< QString, QString >& attributes, 
                              const Secret& secret,
                              bool replace
                            ) :
            CollectionJob( collection, collection ),
            d( new CreateItemJobPrivate( collection->d.data(), collection ) )
{
    d->label = label;
    d->attributes = attributes;
    d->secretPrivate = secret.d;
    d->replace = replace;
}

SecretItem * CreateItemJob::item() const 
{
    return d->item;
}

void CreateItemJob::start()
{
    if ( d->label.length() == 0) {
        finishedWithError( CollectionJob::MissingParameterError, "Please specifify an item propert" );
    }
    else
        startFindCollection();
}

void CreateItemJob::onFindCollectionFinished()
{
    connect( d.data(), SIGNAL(createIsDone( CollectionJob::CollectionError, const QString& )), this, SLOT(createIsDone( CollectionJob::CollectionError, const QString& )) );
    d->startCreateItem();
}

void CreateItemJob::createIsDone( CollectionJob::CollectionError err, const QString& msg )
{
    finishedWithError( err, msg );
}

CreateItemJobPrivate::CreateItemJobPrivate( CollectionPrivate *cp, QObject *parent ) :
        QObject( parent ),
        collectionPrivate( cp )
{
}

void CreateItemJobPrivate::startCreateItem()
{
    QVariantMap varMap;
    varMap["Label"] = label;
    attributes["Label"] = label;
    QVariant varAttrs;
    varAttrs.setValue<StringStringMap>(attributes);
    varMap["Attributes"] = varAttrs;
    SecretStruct secretStruct;
    if ( secretPrivate->toSecretStruct( secretStruct ) ) {
        QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> createReply = collectionPrivate->collectionIf->CreateItem( varMap, secretStruct, replace );
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( createReply );
        connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createItemReply(QDBusPendingCallWatcher*)) );
    }
    else {
        kDebug() << "ERROR preparing SecretStruct";
        emit createIsDone( CollectionJob::CreateError, "Cannot prepare secret structure" );
    }
}

void CreateItemJobPrivate::createItemReply(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> createReply = *watcher;
    if ( !createReply.isError() ) {
        QDBusObjectPath itemPath = createReply.argumentAt<0>();
        QDBusObjectPath promptPath = createReply.argumentAt<1>();
        if ( itemPath.path().compare("/") == 0 ) {
            PromptJob *promptJob = new PromptJob( promptPath, collectionPrivate->promptParentWindowId, this );
            if ( createItemJob->addSubjob( promptJob ) ) {
                connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(createPromptFinished(KJob*)) );
                promptJob->start();
            }
            else {
                kDebug() << "ERROR creating prompt job for " << promptPath.path();
                emit createIsDone( CollectionJob::CreateError, "Cannot create prompt job!" );
            }
        }
        else {
            QSharedPointer< SecretItemPrivate > itemPrivate( new SecretItemPrivate( itemPath ) );
            if ( itemPrivate->isValid() ) {
                item = new SecretItem( itemPrivate );
                emit createIsDone( CollectionJob::NoError, "" );
            }
            else {
                item = NULL;
                kDebug() << "ERROR creating item, as it's invalid. path = " << itemPath.path();
                emit createIsDone( CollectionJob::CreateError, "The backend returned an invalid item path or it's no longer present" );
            }
        }
    }
    else {
        kDebug() << "ERROR trying to create item : " << createReply.error().message();
        emit createIsDone( CollectionJob::CreateError, "Backend communication error" );
    }
    watcher->deleteLater();
}

void CreateItemJobPrivate::createPromptFinished(KJob*)
{
    // TODO: implement this
}


ReadItemsJob::ReadItemsJob( Collection *collection,
                                        QObject *parent ) :
    CollectionJob( collection, parent ),
    d( new ReadItemsJobPrivate( collection->d.data() ) )
{
}

void ReadItemsJob::start()
{
    // this is a property read - Qt seems to read properties synchrounously
    setError( 0 );
    setErrorText( "" );
    emitResult();
}

QList< SecretItem > ReadItemsJob::items() const 
{
    QList< SecretItem > result;
    foreach( SecretItemPrivate ip, d->readItems() ) {
        result.append( SecretItem( new SecretItemPrivate( ip ) ) );
    }
    return result;
}

ReadItemsJobPrivate::ReadItemsJobPrivate( CollectionPrivate *cp ) :
    collectionPrivate( cp )
{
}

QList< SecretItemPrivate > ReadItemsJobPrivate::readItems() const 
{
    QList< SecretItemPrivate > result;
    foreach( QDBusObjectPath path, collectionPrivate->collectionIf->items() ) {
        result.append( SecretItemPrivate( path ) );
    }
    return result;
}


#include "ksecretscollectionjobs.moc"
#include "ksecretscollectionjobs_p.moc"
