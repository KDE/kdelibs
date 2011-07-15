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
        PendingFind     =1,     /// one of the collection methods was called but this object is yet to be connected to the backed
        FoundExisting   =2,     /// this object is connected to an existing backend connection
        NewlyCreated    =3,     /// this object is connected to a newly created connection
        NotFound        =4,     /// the collection was not found
        Deleted         =5      /// the collection was deleted. Calling methods in such a collection would lead to unpredictable results
    };

    /**
     * This will try to find a collection given its name. If not found, it'll create it depending on the
     * options given. Please note that for the sake of asynchronous behaviour, this actual collection finding
     * or creation will be postponed until you'll call one of the methods of the returned object.
     * @param promptParentWindowId identifies the applications window to be used as a parent for prompt windows
     * @param collectionName collection name to be found
     * @param options @see FindCollectionOptions
     * maps to readAlias dbus method
     */
    static Collection * findCollection( const WId &promptParentWindowId,
                                        const QString &collectionName, 
                                        FindCollectionOptions options = CreateCollection );

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
     * @return true if the collection hase been deleted
     */
    KJob * deleteCollection();
    
    /**
     * Change the name of this collection
     * @param newName is the new collection's name
     */
    KJob * renameCollection( const QString& newName );
    
    
    /**
     * Search for the items matching the specified attributes
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
     *
     * @see SecretItem
     */
    CreateItemJob * createItem( const QString& label, const StringStringMap &attributes, const Secret &secret, bool replace =false );

    class ReadItemsJob;
    
    /** 
     * Retrieve items stored inside this collection
     */
    ReadItemsJob * items() const;

    /**
     * Retrieve the lock status of this collection
     * FIXME: should this method be asynchronous?
     */
    bool isLocked() const;
    
    /**
     * Retrieve this collection's label
     * FIXME: should this method be asynchronous?
     */
    QString label() const;
    
    /**
     * Get the creation timestamps of this collection
     * FIXME: should this method be asynchronous?
     */
    QDateTime createdTime() const;
    
    /**
     * Get the last modified timestamp of this collection
     * FIXME: should this method be asynchronous?
     */
    QDateTime modifiedTime() const;

    /**
     * Change this collection's label
     * FIXME: should this method be asynchronous?
     */
    void setLabel( const QString &label );
    
    
Q_SIGNALS:
    void itemCreated( const SecretItem& ); 
    void itemDeleted( const QString& itemLabel );
    void itemChanged( const SecretItem& );

    
protected:
    explicit Collection();
    
private:
    friend class CollectionJob; // to give access to Private class
    friend class FindCollectionJob;
    friend class DeleteCollectionJob;
    friend class RenameCollectionJob;
    friend class CreateItemJob;
    friend class SearchSecretsJob;
    friend class SearchItemsJob;
    
    CollectionPrivate *d;
};



}; // namespace

#endif // KSECRETSSERVICECOLLECTION_H
