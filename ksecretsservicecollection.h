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

#ifndef KSECRETSSERVICECOLLECTION_H
#define KSECRETSSERVICECOLLECTION_H

#include "ksecretsservicesecret.h"
#include "ksecretscollectionjobs.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <kjob.h>
#include <kcompositejob.h>
#include <qwindowdefs.h>

typedef QMap< QString, QString > StringStringMap;

namespace KSecretsService {

class SecretItem;
class CollectionPrivate;
class CreateItemJob;
class SearchSecretsJob;
class SearchItemsJob;
class ReadItemsJob;
class ReadCollectionPropertyJob;
class WriteCollectionPropertyJob;

/**
 * This is the main KSecretsService entry class, used by the applications that need to store secrets, 
 * like passwords or other sensitive data, in a secured and encrypted storage.
 * 
 * All the methods of this class are performing asynchronously. That is, calling them immediatley return 
 * a KJob descendant class. The actual action will be asynchronously done by this job only when it's start()
 * or exec() method is called. Note that exec() method usage is discouraged though.
 * 
 * The client application is responsible for KJob start() or exec() method calling. Upon job finish, the
 * client application should check KJob error() status to know if the requested operation was successufully done.
 * 
 * The KJob descendant classes returned by the methods also provide custom members, depending on the operation
 * they are intented to execute. Upon successuful execution, these members hold the corresponding return values.
 * 
 * @see KJob
 */
class Collection : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Collection)
public:

    /**
     * Options used when findCollection method is called
     */
    enum FindCollectionOptions {
        OpenOnly         =0,    /// this will only try to open the collection without creating it if not found
        CreateCollection =1     /// the collection will be created if not found
    };
    
    enum Status {
        Invalid         =0,     /// the collection objet is freshly initialized and none of it's methods have been called
        Pending         =1,     /// one of the collection methods was called but this object is yet to be connected to the backed
        FoundExisting   =2,     /// this object is connected to an existing backend connection
        NewlyCreated    =3,     /// this object is connected to a newly created connection
        NotFound        =4,     /// the collection was not found
        Deleted         =5      /// the collection was deleted. Calling methods in such a collection would lead to unpredictable results
    };

    /**
     * This will try to find a collection given its name. If not found, it'll create it depending on the
     * options given. Please note that for the sake of asynchronous behaviour, this actual collection finding
     * or creation will be postponed until you'll call one of the methods of the returned object.
     * @param collectionName collection name to be found
     * @param options @see FindCollectionOptions
     * @param promptParentWindowId identifies the applications window to be used as a parent for prompt windows
     * @return Collection instance to be used by the client application to further manipulated it's secrets
     * @note Please note that the collection returned by this method is not yet connected to the secret storing
     * infrastructure. As such, a NotFound status would not be immediatley known. Application should be prepared
     * to get such an error upon the execution of the first KJob returned by one of the other methods of this class.
     */
    static Collection * findCollection( const QString &collectionName, 
                                        FindCollectionOptions options = CreateCollection,
                                        const WId &promptParentWindowId =0 );

    /**
     * This will get the actual findStatus of this collection
     * @return Status
     */
    Status status() const;
    
    /**
     * Try to delete this collection. The user might be prompted to confirm that 
     * and as such he/she may choose not to confirm that operation.
     * 
     * Please note that after successufully calling this method, this object
     * is no longer valid and calling other methods on it would lead to undefined
     * behaviour
     * 
     * @return KJob. Check it's error() member to confirm that the collection hase been deleted
     */
    KJob * deleteCollection();
    
    /**
     * Change the name of this collection
     * @param newName is the new collection's name
     * @return KJob. Check it's error() member to confirm that the rename was effectively done.
     */
    KJob * renameCollection( const QString& newName );
    
    
    /**
     * Search for the items matching the specified attributes
     * @param attributes hold the searched items attributes
     * @return SearchItemsJob which is a KJob inheritor
     */
    SearchItemsJob * searchItems( const StringStringMap &attributes );
    
    /**
     * Use this method to get several secrets without getting through getting items
     */
    SearchSecretsJob * searchSecrets( const StringStringMap &attributes );
    
    /**
     * Create a new item inside the current collection
     * @param label holds the item's label; this label will be automatically added to the attributes map under the index "Label" and it'll eventually replace an existing item on that slot
     * @param attributes holds an map of property names / property values
     * @param secret the secret the newly created item should hold
     * @param replace true if the and eventually existing secret is to be replaced by the on specified here
     * @return CreateItemJob instance used to start the actual item creation operation in the storage infrastructure
     *
     * @see SecretItem
     */
    CreateItemJob * createItem( const QString& label, const StringStringMap &attributes, const Secret &secret, bool replace =false );

    /** 
     * Retrieve items stored inside this collection
     * @return ReadItemJob instance that will hold the returned items upon successuful execution
     */
    ReadItemsJob * items() const;

    /**
     * Retrieve the lock status of this collection
     * @Note returned ReadCollectionPropertyJob::propertyValue is boolean
     */
    ReadCollectionPropertyJob* isLocked() const;
    
    /**
     * Retrieve this collection's label
     * @Note returned ReadCollectionPropertyJob::propertyValue is QString
     */
    ReadCollectionPropertyJob* label() const;
    
    /**
     * Get the creation timestamps of this collection
     * @Note returned ReadCollectionPropertyJob::propertyValue is a time_t
     */
    ReadCollectionPropertyJob* createdTime() const;
    
    /**
     * Get the last modified timestamp of this collection
     * @Note returned ReadCollectionPropertyJob::propertyValue is a time_t
     */
    ReadCollectionPropertyJob* modifiedTime() const;

    /**
     * Change this collection's label
     * @return WriteCollectionPropertyJob instance. Check it's error() status after executing it to confirm collection label change.
     */
    WriteCollectionPropertyJob* setLabel( const QString &label );
    
    /**
     * @return ReadCollectionPropertyJob instance. It's propertyValue() holds the status (true or false) upon job execution finish.
     */
    ReadCollectionPropertyJob* isValid();
    
Q_SIGNALS:
    /**
     * TODO: not yet implemented
     */
    void itemCreated( const SecretItem& ); 
    /**
     * TODO: not yet implemented
     */
    void itemDeleted( const QString& itemLabel );
    /**
     * TODO: not yet implemented
     */
    void itemChanged( const SecretItem& );

    
protected:
    explicit Collection();
    explicit Collection( CollectionPrivate* );
    
private:
    friend class CollectionJob; // to give access to Private class
    friend class FindCollectionJob;
    friend class DeleteCollectionJob;
    friend class RenameCollectionJob;
    friend class CreateItemJob;
    friend class SearchSecretsJob;
    friend class SearchItemsJob;
    friend class ReadItemsJob;
    friend class ReadCollectionPropertyJob;
    friend class WriteCollectionPropertyJob;
    
    void readIsValid( ReadCollectionPropertyJob* );
    
    QSharedPointer< CollectionPrivate > d;
};



}; // namespace

#endif // KSECRETSSERVICECOLLECTION_H
