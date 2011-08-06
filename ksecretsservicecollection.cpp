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

#include "ksecretsservicecollection.h"
#include "ksecretsservicecollection_p.h"
#include "dbusbackend.h"
#include "ksecretsservicecollectionjobs.h"
#include "ksecretsservicecollectionjobs_p.h"
#include "collection_interface.h"

#include <QDateTime>
#include <QtDBus/QDBusPendingReply>
#include <kcompositejob.h>
#include <QTimer>
#include <kjob.h>
#include <kdebug.h>

namespace KSecretsService {

Collection::Collection(): 
        QObject(),
        d( new CollectionPrivate( this ) )
{
    // nothing to do
}

Collection::~Collection()
{
}

Collection * KSecretsService::Collection::findCollection(const QString& collectionName, 
                                                         KSecretsService::Collection::FindCollectionOptions options /* = CreateCollection */,
                                                         const WId &promptParentWindowId /* =0 */ )
{
    // this will simply return the C++ collection objet, without trying to connect to the daemon
    // this will be handled later on, when first calls to other methods will happen
    Collection *collection = new Collection();
    collection->d->setPendingFindCollection( promptParentWindowId, collectionName, options );
    return collection;
}

Collection::Status Collection::status() const
{
    return d->collectionStatus;
}

KJob* Collection::deleteCollection()
{
    return new DeleteCollectionJob( this );
}

KJob* Collection::renameCollection(const QString& newName)
{
    return new RenameCollectionJob( this, newName, this );
}

SearchItemsJob* Collection::searchItems(const QStringStringMap& attributes)
{
    return new SearchItemsJob( this, attributes, this );
}

SearchSecretsJob* Collection::searchSecrets(const QStringStringMap& attributes)
{
    return new SearchSecretsJob( this, attributes, this );
}

CreateItemJob* Collection::createItem(const QString& label, const QMap< QString, QString >& attributes, const Secret& secret, bool replace /* =false */)
{
    return new CreateItemJob( this, label, attributes, secret, replace );
}

ReadItemsJob* Collection::items() const
{
    return new ReadItemsJob( const_cast< Collection* >(this) );
}

ReadCollectionPropertyJob* Collection::isLocked() const
{
    return new ReadCollectionPropertyJob( const_cast< Collection* >(this), "Locked" );
}

ReadCollectionPropertyJob* Collection::label() const
{
    return new ReadCollectionPropertyJob( const_cast< Collection* >(this), "Label" );
}

ReadCollectionPropertyJob* Collection::createdTime() const
{
    return new ReadCollectionPropertyJob( const_cast< Collection* >(this), "Created" );
}

ReadCollectionPropertyJob* Collection::modifiedTime() const
{
    return new ReadCollectionPropertyJob( const_cast< Collection* >(this), "Modified" );
}

WriteCollectionPropertyJob* Collection::setLabel(const QString& label)
{
    return new WriteCollectionPropertyJob( this, "Label", QVariant( label ) );
}



CollectionPrivate::CollectionPrivate( Collection *coll ) :
        collection( coll ),
        findOptions( Collection::OpenOnly ),
        collectionStatus( Collection::Invalid ),
        collectionIf( 0 )
{
}

CollectionPrivate::~CollectionPrivate()
{
}

void CollectionPrivate::setPendingFindCollection( const WId &promptParentId,
                                                  const QString &collName, 
                                                  Collection::FindCollectionOptions opts ) 
{
    collectioName = collName;
    findOptions = opts;
    collectionStatus = Collection::Pending;
    promptParentWindowId = promptParentId;
}

bool CollectionPrivate::isValid()
{
    // NOTE: do not call collectionInterface() to get the interface pointer, if not you'll get an infinite recursive call
    return 
        collectionIf && collectionIf->isValid() && (
            collectionStatus == Collection::FoundExisting ||
            collectionStatus == Collection::NewlyCreated );
}

void CollectionPrivate::setDBusPath( const QDBusObjectPath &path )
{
    collectionIf = DBusSession::createCollection( path );
    if ( collectionIf->isValid() ) {
        collectionStatus = (findOptions & Collection::CreateCollection) ? Collection::NewlyCreated : Collection::FoundExisting;
        kDebug() << "SUCCESS opening collection " << path.path();
    }
    else {
        collectionStatus = Collection::NotFound;
        kDebug() << "ERROR opening collection " << path.path();
    }
}

const WId & CollectionPrivate::promptParentId() const 
{
    return promptParentWindowId;
}

OrgFreedesktopSecretCollectionInterface *CollectionPrivate::collectionInterface()
{
    if ( (collectionIf == 0) || ( !collectionIf->isValid() ) ) {
        // well, some attribute read method is now happening and we should now really open or find the collection
        // the only problem is that we'll be forced to call findJob->exec() to do this and it's evil :-)
        FindCollectionJob *findJob = new FindCollectionJob( collection, 0 );
        findJob->exec();
    }
    return collectionIf;
}

ReadCollectionPropertyJob* Collection::isValid()
{
    return new ReadCollectionPropertyJob( this, &Collection::readIsValid, this );
}

void Collection::readIsValid( ReadCollectionPropertyJob *readPropertyJob)
{
    readPropertyJob->d->value = d->isValid();
}

#include "ksecretsservicecollection.moc"

    
};

