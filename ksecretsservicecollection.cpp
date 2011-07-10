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
#include "ksecretscollectionjobs.h"
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
        d( new CollectionPrivate() )
{
    // nothing to do
}

Collection * KSecretsService::Collection::findCollection(const WId &promptParentWindowId,
                                                         const QString& collectionName, 
                                                         KSecretsService::Collection::FindCollectionOptions options)
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

KJob* Collection::searchItems(const QStringStringMap& attributes)
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

Collection::ReadItemsJob* Collection::items() const
{
    // TODO: implement this
    Q_ASSERT(0);
    return NULL;
}

bool Collection::isLocked() const
{
    // TODO: implement this
    Q_ASSERT(0);
    return false;
}

QString Collection::label() const
{
    return d->collectionIf->property( "Label" ).toString();
}

QDateTime Collection::createdTime() const
{
    // TODO: implement this
    Q_ASSERT(0);
    return QDateTime();
}

QDateTime Collection::modifiedTime() const
{
    // TODO: implement this
    Q_ASSERT(0);
    return QDateTime();
}

void Collection::setLabel(const QString& label)
{
    // TODO: implement this
    Q_ASSERT(0);
}



CollectionPrivate::CollectionPrivate() :
        findOptions( Collection::OpenOnly ),
        collectionStatus( Collection::Invalid )
{
}

void CollectionPrivate::setPendingFindCollection( const WId &promptParentId,
                                                  const QString &collName, 
                                                  Collection::FindCollectionOptions opts ) 
{
    collectioName = collName;
    findOptions = opts;
    collectionStatus = Collection::PendingFind;
    promptParentWindowId = promptParentId;
}

bool CollectionPrivate::isValid() const 
{
    return 
        collectionStatus == Collection::FoundExisting ||
        collectionStatus == Collection::NewlyCreated;
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

#include "ksecretsservicecollection.moc"

    
};

