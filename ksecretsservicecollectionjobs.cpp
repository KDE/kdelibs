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

#include "ksecretsservicecollectionjobs.h"
#include "ksecretsservicecollectionjobs_p.h"
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
#include <QSharedDataPointer>
#include <kdebug.h>
#include <prompt_interface.h>
#include <kapplication.h>
#include <QWidget>
#include <klocalizedstring.h>

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

CollectionJob::~CollectionJob()
{
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
    d->collectionName = collection->d->collectionName;
    d->findCollectionOptions = collection->d->findOptions;
}

FindCollectionJob::~FindCollectionJob()
{
}

void FindCollectionJob::start() 
{
    // meanwhile another findJob instance would have already connected our collection object
    if ( ! collection()->d->isValid() ) {
        d->startCreateOrOpenCollection();
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
            Q_ASSERT( promptPath.path().compare("/") ); // we should have a prompt path here other thant "/"
            PromptJob *promptJob = new PromptJob( promptPath, collectionPrivate->promptParentId(), this );
            if ( findJob->addSubjob( promptJob ) ) {
                connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(createPromptFinished(KJob*)) );
                promptJob->start();
            }
            else {
                promptJob->deleteLater();
                kDebug() << "cannot add prompt subjob!";
                findJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot add prompt job") );
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
            findJob->d->collectionPrivate->setDBusPath( collPath );
            findJob->finishedOk();
        }
        else {
            findJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, i18n("The operation was cancelled by the user") );
        }
    }
    else {
        findJob->finishedWithError( CollectionJob::InternalError, i18n("Error encountered when trying to prompt the user") );
    }
    job->deleteLater();
}


void FindCollectionJobPrivate::startCreateOrOpenCollection()
{
    OpenSessionJob *openSessionJob = DBusSession::openSession();
    if ( findJob->addSubjob( openSessionJob ) ) {
        connect( openSessionJob, SIGNAL(finished(KJob*)), this, SLOT(openSessionFinished(KJob*)) );
        openSessionJob->start();
    }
    else {
        kDebug() << "Cannot OpenSessionJob subjob";
        findJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot open session") );
    }
}

void FindCollectionJobPrivate::openSessionFinished(KJob* theJob)
{
    if ( !theJob->error() ) {
        if ( collectionPrivate->findOptions == Collection::CreateCollection ) {
            OpenSessionJob *openSessionJob = dynamic_cast< OpenSessionJob * >( theJob );
            QVariantMap creationProperties = collectionPrivate->collectionProperties;
            creationProperties.insert("org.freedesktop.Secret.Collection.Label", collectionName);
            QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > createReply = openSessionJob->serviceInterface()->CreateCollection(
                creationProperties, collectionName );
            QDBusPendingCallWatcher *createReplyWatch = new QDBusPendingCallWatcher( createReply, this );
            connect( createReplyWatch, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createFinished(QDBusPendingCallWatcher*)) );
        }
        else 
            if ( collectionPrivate->findOptions == Collection::OpenOnly ) {
                QList< QDBusObjectPath > collPaths = DBusSession::serviceIf()->collections();
                foreach ( const QDBusObjectPath &collPath, collPaths ) {
                    OrgFreedesktopSecretCollectionInterface *coll = DBusSession::createCollection( collPath );
                    coll->deleteLater();
                    if ( coll->label() == collectionName ) {
                        findJob->d->collectionPrivate->setDBusPath( collPath );
                        findJob->finishedOk();
                        break;
                    }
                }
            }
            else {
                Q_ASSERT(0);
            }
    }
}


ListCollectionsJob::ListCollectionsJob( Collection * coll ) :
    CollectionJob( coll, coll ),
    d( new ListCollectionsJobPrivate( this, coll->d.data() ) )
{
}

ListCollectionsJob::~ListCollectionsJob()
{
}

void ListCollectionsJob::start()
{
    d->startListingCollections();
}

const QStringList &ListCollectionsJob::collections() const 
{
    return d->collections;
}

ListCollectionsJobPrivate::ListCollectionsJobPrivate( ListCollectionsJob *job, CollectionPrivate *cp ) :
    listCollectionsJob(job),
    collectionPrivate(cp)
{
}

void ListCollectionsJobPrivate::startListingCollections()
{
    QList<QDBusObjectPath> collPaths = DBusSession::serviceIf()->collections();
    foreach( const QDBusObjectPath &path, collPaths ) {
        OrgFreedesktopSecretCollectionInterface *coll = DBusSession::createCollection( path );
        collections.append( coll->label() );
        coll->deleteLater();
    }
    listCollectionsJob->finishedOk();
}

DeleteCollectionJob::DeleteCollectionJob( Collection* collection, QObject* parent ) :
        CollectionJob( collection, parent),
        d( new DeleteCollectionJobPrivate( collection->d.data(), this ) )
{
}
    
DeleteCollectionJob::~DeleteCollectionJob()
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
    QDBusPendingReply<QDBusObjectPath> deleteReply = cp->collectionInterface()->Delete();
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

RenameCollectionJob::~RenameCollectionJob()
{
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
    if ( collectionPrivate->collectionInterface()->setProperty( "Label", QVariant( newName ) ) ) {
        emit renameIsDone( CollectionJob::NoError, "" );
    }
    else {
        emit renameIsDone( CollectionJob::RenameError, QString( "Cannot rename collection to %1" ).arg( newName ) );
    }
}

SearchCollectionItemsJob::SearchCollectionItemsJob( Collection *collection, 
                                const QStringStringMap &attributes,
                                QObject *parent ) :
    CollectionJob( collection, parent ),
    d( new SearchCollectionItemsJobPrivate( collection->d.data(), this ) )
{
    d->attributes = attributes;
}

SearchCollectionItemsJob::~SearchCollectionItemsJob()
{
}

void SearchCollectionItemsJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

QList< QExplicitlySharedDataPointer< SecretItem > > SearchCollectionItemsJob::items() const
{
    QList< QExplicitlySharedDataPointer< SecretItem > > items;
    foreach( QSharedDataPointer< SecretItemPrivate > ip, d->items ) {
        items.append( QExplicitlySharedDataPointer< SecretItem>( new SecretItem(  ip ) ) );
    }
    return items;
}

void SearchCollectionItemsJob::onFindCollectionFinished()
{
    d->startSearchItems();
}

SearchCollectionItemsJobPrivate::SearchCollectionItemsJobPrivate( CollectionPrivate* cp, SearchCollectionItemsJob *job ) :
    QObject( job ),
    collectionPrivate( cp ),
    searchItemJob( job )
{
}

void SearchCollectionItemsJobPrivate::startSearchItems()
{
    QDBusPendingReply< QList< QDBusObjectPath > > reply = collectionPrivate->collectionInterface()->SearchItems( attributes );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(searchFinished(QDBusPendingCallWatcher*) ) );
}

void SearchCollectionItemsJobPrivate::searchFinished(QDBusPendingCallWatcher* watcher)
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply< QList< QDBusObjectPath > > reply = *watcher;
    if ( !reply.isError() ) {
        QList< QDBusObjectPath > itemList = reply.argumentAt<0>();
        foreach( QDBusObjectPath itemPath, itemList ) {
            items.append( QSharedDataPointer<SecretItemPrivate>( new SecretItemPrivate( itemPath ) ) );
        }
        searchItemJob->finishedOk();
    }
    else {
        kDebug() << "ERROR searching items";
        searchItemJob->finishedWithError( CollectionJob::InternalError, i18n("ERROR searching items") );
    }
    watcher->deleteLater();
}


SearchCollectionSecretsJob::SearchCollectionSecretsJob( Collection* collection, const QStringStringMap &attributes, QObject* parent ) : 
    CollectionJob( collection, parent ),
    d( new SearchCollectionSecretsJobPrivate( collection->d.data(), attributes ) )
{
}

SearchCollectionSecretsJob::~SearchCollectionSecretsJob()
{
}

QList< Secret > SearchCollectionSecretsJob::secrets() const
{
    QList< Secret > result;
    foreach( QSharedDataPointer< SecretPrivate > sp, d->secretsList ) {
        result.append( Secret( sp ) );
    }
    return result;
}

void SearchCollectionSecretsJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

void SearchCollectionSecretsJob::onFindCollectionFinished()
{
    connect( d.data(), SIGNAL(searchIsDone( CollectionJob::CollectionError, const QString& )), this, SLOT(searchIsDone( CollectionJob::CollectionError, const QString&)) );
    d->startSearchSecrets();
}

void SearchCollectionSecretsJob::searchIsDone( CollectionJob::CollectionError err, const QString& msg)
{
    finishedWithError( err, msg );
}

SearchCollectionSecretsJobPrivate::SearchCollectionSecretsJobPrivate( CollectionPrivate *cp, const QStringStringMap &attrs, QObject *parent ) :
    QObject( parent ),
    collectionPrivate( cp ),
    attributes( attrs )
{
}

void SearchCollectionSecretsJobPrivate::startSearchSecrets()
{
    QDBusPendingReply<QList<QDBusObjectPath> > searchReply = collectionPrivate->collectionInterface()->SearchItems( attributes );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( searchReply, this );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(searchSecretsReply(QDBusPendingCallWatcher*)));
}

void SearchCollectionSecretsJobPrivate::searchSecretsReply( QDBusPendingCallWatcher *watcher )
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

void SearchCollectionSecretsJobPrivate::getSecretsReply(QDBusPendingCallWatcher* watcher)
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply<ObjectPathSecretMap> getReply = *watcher;
    if ( !getReply.isError() ) {
        foreach (SecretStruct secret, getReply.value()) {
            SecretPrivate *sp =0;
            if ( SecretPrivate::fromSecretStrut( secret, sp ) ) {
                secretsList.append( QSharedDataPointer<SecretPrivate>( sp ) );
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


CreateCollectionItemJob::CreateCollectionItemJob( Collection *collection,
                              const QString& label,
                              const QMap< QString, QString >& attributes, 
                              const Secret& secret,
                              bool replace
                            ) :
            CollectionJob( collection, collection ),
            d( new CreateCollectionItemJobPrivate( collection->d.data(), collection ) )
{
    d->createItemJob = this;
    d->label = label;
    d->attributes = attributes;
    d->secretPrivate = secret.d;
    d->replace = replace;
}

CreateCollectionItemJob::~CreateCollectionItemJob()
{
}

SecretItem * CreateCollectionItemJob::item() const 
{
    return d->item;
}

void CreateCollectionItemJob::start()
{
    if ( d->label.length() == 0) {
        finishedWithError( CollectionJob::MissingParameterError, i18n("Please specify an item properly") );
    }
    else
        startFindCollection();
}

void CreateCollectionItemJob::onFindCollectionFinished()
{
    connect( d.data(), SIGNAL(createIsDone( CollectionJob::CollectionError, const QString& )), this, SLOT(createIsDone( CollectionJob::CollectionError, const QString& )) );
    d->startCreateItem();
}

// void CreateCollectionItemJob::createIsDone( CollectionJob::CollectionError err, const QString& msg )
// {
//     finishedWithError( err, msg );
// }

CreateCollectionItemJobPrivate::CreateCollectionItemJobPrivate( CollectionPrivate *cp, QObject *parent ) :
        QObject( parent ),
        collectionPrivate( cp )
{
}

void CreateCollectionItemJobPrivate::startCreateItem()
{
    QVariantMap varMap;
    varMap["Label"] = label;
    attributes["Label"] = label;
    QVariant varAttrs;
    varAttrs.setValue<StringStringMap>(attributes);
    varMap["Attributes"] = varAttrs;
    SecretStruct secretStruct;
    if ( secretPrivate->toSecretStruct( secretStruct ) ) {
        QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> createReply = collectionPrivate->collectionInterface()->CreateItem( varMap, secretStruct, replace );
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( createReply );
        connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createItemReply(QDBusPendingCallWatcher*)) );
    }
    else {
        kDebug() << "ERROR preparing SecretStruct";
        createItemJob->finishedWithError( CollectionJob::CreateError, i18n("Cannot prepare secret structure") );
    }
}

void CreateCollectionItemJobPrivate::createItemReply(QDBusPendingCallWatcher* watcher)
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
                createItemJob->finishedWithError( CollectionJob::CreateError, i18n("Cannot create prompt job!") );
            }
        }
        else {
            QSharedDataPointer< SecretItemPrivate > itemPrivate( new SecretItemPrivate( itemPath ) );
            if ( itemPrivate->isValid() ) {
                item = new SecretItem( itemPrivate );
                createItemJob->finishedOk();
            }
            else {
                item = NULL;
                kDebug() << "ERROR creating item, as it's invalid. path = " << itemPath.path();
                createItemJob->finishedWithError( CollectionJob::CreateError, i18n("The backend returned an invalid item path or it's no longer present") );
            }
        }
    }
    else {
        kDebug() << "ERROR trying to create item : " << createReply.error().message();
        createItemJob->finishedWithError( CollectionJob::CreateError, i18n("Backend communication error") );
    }
    watcher->deleteLater();
}

void CreateCollectionItemJobPrivate::createPromptFinished(KJob*)
{
    // TODO: implement this
}


ReadCollectionItemsJob::ReadCollectionItemsJob( Collection *collection,
                                        QObject *parent ) :
    CollectionJob( collection, parent ),
    d( new ReadCollectionItemsJobPrivate( collection->d.data() ) )
{
}

ReadCollectionItemsJob::~ReadCollectionItemsJob()
{
}

void ReadCollectionItemsJob::start()
{
    // this is a property read - Qt seems to read properties synchrounously
    setError( 0 );
    setErrorText( "" );
    emitResult();
}

QList< QExplicitlySharedDataPointer< SecretItem > > ReadCollectionItemsJob::items() const 
{
    QList< QExplicitlySharedDataPointer< SecretItem > > result;
    foreach( QSharedDataPointer< SecretItemPrivate > ip, d->readItems() ) {
        result.append( QExplicitlySharedDataPointer< SecretItem>( new SecretItem( ip ) ) );
    }
    return result;
}

ReadCollectionItemsJobPrivate::ReadCollectionItemsJobPrivate( CollectionPrivate *cp ) :
    collectionPrivate( cp )
{
}

QList< QSharedDataPointer< SecretItemPrivate > > ReadCollectionItemsJobPrivate::readItems() const 
{
    QList< QSharedDataPointer< SecretItemPrivate > > result;
    foreach( QDBusObjectPath path, collectionPrivate->collectionInterface()->items() ) {
        result.append( QSharedDataPointer<SecretItemPrivate>( new SecretItemPrivate( path ) ) );
    }
    return result;
}

ReadCollectionPropertyJob::ReadCollectionPropertyJob( Collection *coll, const char *propName, QObject *parent ) :
    CollectionJob( coll, parent ),
    d( new ReadCollectionPropertyJobPrivate( coll->d.data(), this ) ),
    propertyReadMember(0)
{
    d->propertyName = propName;
}

ReadCollectionPropertyJob::~ReadCollectionPropertyJob()
{
}

ReadCollectionPropertyJob::ReadCollectionPropertyJob( Collection *coll, void (Collection::*propReadMember)( ReadCollectionPropertyJob* ), QObject *parent ) :
    CollectionJob( coll, parent ),
    d( new ReadCollectionPropertyJobPrivate( coll->d.data(), this ) ),
    propertyReadMember( propReadMember )
{
}

void ReadCollectionPropertyJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

void ReadCollectionPropertyJob::onFindCollectionFinished()
{
    if ( propertyReadMember ) {
        (collection()->*propertyReadMember)( this );
        finishedOk();
    }
    else {
        d->startReadingProperty();
    }
}

const QVariant& ReadCollectionPropertyJob::propertyValue() const
{
    return d->value;
}

ReadCollectionPropertyJobPrivate::ReadCollectionPropertyJobPrivate( CollectionPrivate *cp, ReadCollectionPropertyJob *job ) :
    collectionPrivate( cp ),
    readPropertyJob( job )
{
}
    
void ReadCollectionPropertyJobPrivate::startReadingProperty()
{
    value = collectionPrivate->collectionInterface()->property( propertyName );
    readPropertyJob->finishedOk();
}


WriteCollectionPropertyJob::WriteCollectionPropertyJob( Collection *coll, const char *propName, const QVariant& value, QObject *parent ) :
    CollectionJob( coll, parent ),
    d( new WriteCollectionPropertyJobPrivate( coll->d.data(), this ) )
{
    d->propertyName = propName;
    d->value = value;
}

WriteCollectionPropertyJob::~WriteCollectionPropertyJob()
{
}

void WriteCollectionPropertyJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

void WriteCollectionPropertyJob::onFindCollectionFinished()
{
    d->startWritingProperty();
}

WriteCollectionPropertyJobPrivate::WriteCollectionPropertyJobPrivate( CollectionPrivate *cp, WriteCollectionPropertyJob *job ) :
    collectionPrivate( cp ),
    writePropertyJob( job )
{
}
    
void WriteCollectionPropertyJobPrivate::startWritingProperty()
{
    value = collectionPrivate->collectionInterface()->setProperty( propertyName, value );
    writePropertyJob->finishedOk();
}

ChangeCollectionPasswordJob::ChangeCollectionPasswordJob(Collection* collection): 
    CollectionJob( collection ),
    d( new ChangeCollectionPasswordJobPrivate( collection->d.data(), this ) )
{
}

void ChangeCollectionPasswordJob::start()
{
    startFindCollection();
}

void ChangeCollectionPasswordJob::onFindCollectionFinished()
{
    d->startChangingPassword();
}

ChangeCollectionPasswordJobPrivate::ChangeCollectionPasswordJobPrivate( CollectionPrivate *cp, ChangeCollectionPasswordJob *job ) :
    collectionPrivate( cp ),
    theJob( job )
{
}

void ChangeCollectionPasswordJobPrivate::startChangingPassword()
{
    QDBusPendingReply< QDBusObjectPath > reply = collectionPrivate->collectionInterface()->ChangePassword();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(changePasswordStarted(QDBusPendingCallWatcher*)) );
}

void ChangeCollectionPasswordJobPrivate::changePasswordStarted( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply< QDBusObjectPath > reply = *watcher;
    if ( !reply.isError() ) {
        QDBusObjectPath promptPath = reply.argumentAt<0>();
        PromptJob *promptJob = new PromptJob( promptPath, collectionPrivate->promptParentId(), this );
        if ( theJob->addSubjob( promptJob ) ) {
            connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(promptFinished(KJob*)) );
            promptJob->start();
        }
        else {
            promptJob->deleteLater();
            kDebug() << "cannot add prompt subjob!";
            theJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot add prompt job") );
        }
    }
    else {
        kDebug() << "ERROR when starting password change " << reply.error().message();
        theJob->finishedWithError( CollectionJob::InternalError, reply.error().message() );
    }
    watcher->deleteLater();
}

void ChangeCollectionPasswordJobPrivate::promptFinished( KJob* pj )
{
    PromptJob *promptJob = dynamic_cast< PromptJob* >( pj );
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            theJob->finishedOk();
        }
        else {
            theJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, i18n("The operation was cancelled by the user") );
        }
    }
    else {
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Error encountered when trying to prompt the user") );
    }
    pj->deleteLater();
}


CollectionLockJob::CollectionLockJob( Collection *coll, const WId winId ) :
    CollectionJob( coll ),
    d( new CollectionLockJobPrivate( coll->d.data(), this ) )
{
    d->windowId = winId;
}

void CollectionLockJob::start()
{
    startFindCollection();
}

void CollectionLockJob::onFindCollectionFinished()
{
    d->startLockingCollection();
}

CollectionLockJobPrivate::CollectionLockJobPrivate( CollectionPrivate *cp, CollectionLockJob *j ) :
    collectionPrivate( cp ),
    theJob( j )
{
}

void CollectionLockJobPrivate::startLockingCollection()
{
    QList< QDBusObjectPath > lockList;
    lockList.append( QDBusObjectPath( collectionPrivate->collectionInterface()->path() ) );
    QDBusPendingReply<QList<QDBusObjectPath> , QDBusObjectPath> reply = DBusSession::serviceIf()->Lock( lockList );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(slotLockFinished(QDBusPendingCallWatcher*)) );
}

void CollectionLockJobPrivate::slotLockFinished( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT(watcher);
    QDBusPendingReply<QList<QDBusObjectPath> , QDBusObjectPath> reply = *watcher;
    if ( !reply.isError() ) {
        QDBusObjectPath promptPath = reply.argumentAt<1>();
        if ( promptPath.path().compare("/") ) {
            PromptJob *promptJob = new PromptJob( promptPath, windowId, this ); 
            connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(slotPromptFinished(KJob*)) );
            if ( theJob->addSubjob( promptJob ) ) {
                promptJob->start();
            }
            else {
                promptJob->deleteLater();
                kDebug() << "cannot add prompt subjob";
                theJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot add prompt job") );
            }
        }
        else {
            QList< QDBusObjectPath > objList = reply.argumentAt<0>();
            checkResult( objList );
        }
    }
    else {
        kDebug() << "ERROR when trying to lock collection " << reply.error().message();
        theJob->finishedWithError( CollectionJob::InternalError, reply.error().message() );
    }
    watcher->deleteLater();
}

void CollectionLockJobPrivate::slotPromptFinished( KJob* j )
{
    PromptJob *promptJob = qobject_cast< PromptJob* >(j);
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            QDBusVariant res = promptJob->result();
            if (res.variant().canConvert< QList< QDBusObjectPath> >()) {
                QList< QDBusObjectPath > objList = res.variant().value< QList< QDBusObjectPath > >();
                checkResult( objList );
            }
            else {
                theJob->finishedWithError( CollectionJob::InternalError, i18n("Unlock operation returned unexpected result") );
            }
        }
        else {
            theJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, i18n("The operation was cancelled by the user") );
        }
    }
    else {
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Error encountered when trying to prompt the user") );
    }
}

void CollectionLockJobPrivate::checkResult( const QList< QDBusObjectPath > & objList ) const
{
    if ( objList.count() == 1 && objList.first().path() == collectionPrivate->collectionInterface()->path() ) {
        theJob->finishedOk();
    }
    else {
        kDebug() << "objList.count() = " << objList.count();
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Unlock operation returned unexpected result") );
    }
}

#include "ksecretsservicecollectionjobs.moc"
#include "ksecretsservicecollectionjobs_p.moc"
