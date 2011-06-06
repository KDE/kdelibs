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

#ifndef KSECRETSCOLLECTION_H
#define KSECRETSCOLLECTION_H

#include "ksecretsservicesecret.h"
#include "ksecretsservicejob.h"

#include <QObject>
#include <QMap>
#include <kjob.h>

namespace KSecretsService {

class SecretItem;

typedef QMap< QString, QString > QStringStringMap;

class Collection : public QObject {
    Q_OBJECT
public:

    /**
     * Options used when findCollection method is called
     */
    enum FindCollectionOptions {
        OpenOnly         =0,    /// this will only try to open the collection without creating it if not found
        CreateCollection =1     /// the collection will be created if not found
    };
    
    /**
     * This will try to find a collection given its name. If not found, it'll create it depending on the
     * options given. Please note that for the sake of asynchronous behaviour, this actual collection finding
     * or creation will be postponed until you'll call one of the methods of the returned object.
     * @param collectionName collection name to be found
     * @param options @see FindCollectionOptions
     * maps to readAlias dbus method
     */
    static Collection * findCollection( const QString &collectionName, 
                                               FindCollectionOptions options = CreateCollection );
    
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
    
    
    class SearchItemsJob;
    
    /**
     * Search for the items matching the specified attributes
     */
    SearchItemsJob * searchItems( const QStringStringMap &attributes );
    
    class SearchSecretsJob;
    
    /**
     * Use this method to get several secrets without getting through getting items
     */
    SearchSecretsJob * searchSecrets( const QStringStringMap &attributes );
    
    class CreateItemJob;
    /**
     * Create a new item inside the current collection
     * @param properties holds an map of property names / property values
     * @param secret the secret the newly created item should hold
     */
    CreateItemJob * createItem( const QStringStringMap &attributes, const Secret &secret );

    class ReadItemsJob;
    
    /** 
     * Retrieve items stored inside this collection
     */
    ReadItemsJob * items() const;

    /**
     * Retrieve the lock status of this collection
     * FIXME: should this method be asynchronous
     */
    bool isLocked() const;
    
    /**
     * Retrieve this collection's label
     * FIXME: should this method be asynchronous
     */
    QString label() const;
    
    /**
     * Get the creation timestamps of this collection
     * FIXME: should this method be asynchronous
     */
    QDateTime createdTime() const;
    
    /**
     * Get the last modified timestamp of this collection
     * FIXME: should this method be asynchronous
     */
    QDateTime modifiedTime() const;

    /**
     * Change this collection's label
     * FIXME: should this method be asynchronous
     */
    void setLabel( const QString &label );
    
Q_SIGNALS:
    void itemCreated( const SecretItem& ); 
    void itemDeleted( const QString& itemLabel );
    void itemChanged( const SecretItem& );
    
protected:
    
private:
    
    class Private; 
    Private *d;
};


class Collection::SearchItemsJob : public SecretsJobBase {
    Q_OBJECT
    Q_DISABLE_COPY(SearchItemsJob)
public:
    explicit SearchItemsJob( QObject *parent );
    
    QList< SecretItem > &items() const;
    
private:
    class Private;
    Private *d;
};

class Collection::SearchSecretsJob : public SecretsJobBase {
    Q_OBJECT
    Q_DISABLE_COPY(SearchSecretsJob)
public:
    explicit SearchSecretsJob( QObject* parent );
    
    QList< Secret >  secrets() const;
    
private:
    class Private;
    Private *d;
};

class Collection::CreateItemJob : public SecretsJobBase {
    Q_OBJECT
    Q_DISABLE_COPY(CreateItemJob)
public:
    explicit CreateItemJob( QObject *parent );
    
    SecretItem item() const;
    
private:
    class Private;
    Private *d;
};

class Collection::ReadItemsJob : public SecretsJobBase {
    Q_OBJECT
    Q_DISABLE_COPY(ReadItemsJob)
public:
    explicit ReadItemsJob( QObject *parent );
    
    QList< SecretItem* > items() const;
    
private:
    class Private;
    Private *d;
};

}; // namespace

#endif // KSECRETSCOLLECTION_H
