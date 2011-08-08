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

#ifndef KSECRETSSERVICECOLLECTIONJOBS_P_H
#define KSECRETSSERVICECOLLECTIONJOBS_P_H

#include "ksecretsservicecollectionjobs.h"
#include "../daemon/frontend/secret/adaptors/secretstruct.h"

#include <QObject>
#include <QDBusPendingReply>

namespace KSecretsService {
class SecretItem;

class OrgFreedesktopSecretPromptInterface;
class SecretPrivate;

class CollectionJobPrivate {
public:
    CollectionJobPrivate();
    
    Collection      *collection;
};

/**
 * Internal DeleteCollectionJob implementation, based on DBus
 */
class DeleteCollectionJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(DeleteCollectionJobPrivate);
public:
    explicit DeleteCollectionJobPrivate( CollectionPrivate *coll, QObject *parent =0 );

    void startDelete();
    
    friend class DeleteCollectionJob;
    
Q_SIGNALS:
    void deleteIsDone( CollectionJob::CollectionError error, const QString& message );
    
protected Q_SLOTS:
    void callFinished( QDBusPendingCallWatcher* );

public:
    CollectionPrivate    *cp;
};

class FindCollectionJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(FindCollectionJobPrivate)
public:
    friend class FindCollectionJob;
    
    FindCollectionJobPrivate( FindCollectionJob*, CollectionPrivate* );

    void startCreateOrOpenCollection();
    
private Q_SLOTS:
    void openSessionFinished(KJob*);
    void createFinished(QDBusPendingCallWatcher*);
    void createPromptFinished(KJob*);
    
public:
    FindCollectionJob                                    *findJob;
    QString                                              collectionName;
    KSecretsService::Collection::FindCollectionOptions   findCollectionOptions;
    CollectionPrivate                                    *collectionPrivate;
};

class ListCollectionsJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(ListCollectionsJobPrivate)
public:
    ListCollectionsJobPrivate( ListCollectionsJob*, CollectionPrivate* );
    
    void startListingCollections();
    
public:
    ListCollectionsJob  *listCollectionsJob;
    CollectionPrivate   *collectionPrivate;
    QStringList         collections;
    
};

class RenameCollectionJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(RenameCollectionJobPrivate)
public:
    friend class RenameCollectionJob;
    
    explicit RenameCollectionJobPrivate( CollectionPrivate*, QObject *parent =0 );
    void startRename();

Q_SIGNALS:
    void renameIsDone( CollectionJob::CollectionError error, const QString& message );
    
public:
    CollectionPrivate   *collectionPrivate;
    QString             newName;
};

class SearchCollectionItemsJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SearchCollectionItemsJobPrivate)
public:
    SearchCollectionItemsJobPrivate( CollectionPrivate*, SearchCollectionItemsJob* );
    
    void startSearchItems();
    
Q_SIGNALS:
    void searchIsDone(CollectionJob::CollectionError, const QString& );

protected Q_SLOTS:
    void searchFinished(QDBusPendingCallWatcher*);
    
public:
    CollectionPrivate                   *collectionPrivate;
    SearchCollectionItemsJob            *searchItemJob;
    QStringStringMap                    attributes;
    QList< QSharedDataPointer< SecretItemPrivate > >    items;
};

class CreateCollectionItemJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(CreateCollectionItemJobPrivate)
public:
    explicit CreateCollectionItemJobPrivate( CollectionPrivate*, QObject *parent =0 );

    void startCreateItem();
    
protected Q_SLOTS:
    void createItemReply(QDBusPendingCallWatcher*);
    void createPromptFinished(KJob*);
    
public:
    CollectionPrivate                   *collectionPrivate;
    CreateCollectionItemJob                       *createItemJob;
    QString                             label;
    QMap< QString, QString >            attributes;
    QSharedDataPointer< SecretPrivate > secretPrivate;
    bool                                replace;
    SecretItem                          *item;
};

class SearchCollectionSecretsJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SearchCollectionSecretsJobPrivate)
public:
    explicit SearchCollectionSecretsJobPrivate( CollectionPrivate*, const QStringStringMap &attrs, QObject *parent =0);
    
    void startSearchSecrets();
    
protected Q_SLOTS:
    void searchSecretsReply(QDBusPendingCallWatcher*);
    void getSecretsReply(QDBusPendingCallWatcher*);
    
Q_SIGNALS:
    void searchIsDone( CollectionJob::CollectionError, const QString& );
    
public:
    CollectionPrivate   *collectionPrivate;
    QStringStringMap    attributes;
    QList< QSharedDataPointer<SecretPrivate> > secretsList;
};

class ReadCollectionItemsJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(ReadCollectionItemsJobPrivate)
public:
    explicit ReadCollectionItemsJobPrivate( CollectionPrivate* );

    QList< QSharedDataPointer< SecretItemPrivate > > readItems() const;
    
public:
    CollectionPrivate   *collectionPrivate;
};

class ReadCollectionPropertyJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(ReadCollectionPropertyJobPrivate)
public:
    ReadCollectionPropertyJobPrivate( CollectionPrivate* cp, ReadCollectionPropertyJob* );
    
    void startReadingProperty();
    
    CollectionPrivate *collectionPrivate;
    ReadCollectionPropertyJob *readPropertyJob;
    const char *propertyName;
    QVariant value;
};

class WriteCollectionPropertyJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(WriteCollectionPropertyJobPrivate)
public:
    WriteCollectionPropertyJobPrivate( CollectionPrivate *cp, WriteCollectionPropertyJob* );
    
    void startWritingProperty();
    
    CollectionPrivate *collectionPrivate;
    WriteCollectionPropertyJob *writePropertyJob;
    const char *propertyName;
    QVariant value;
};

class ChangeCollectionPasswordJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(ChangeCollectionPasswordJobPrivate)
public:
    explicit ChangeCollectionPasswordJobPrivate( CollectionPrivate *cp, ChangeCollectionPasswordJob* );
    
    void startChangingPassword();

protected Q_SLOTS:
    void changePasswordStarted( QDBusPendingCallWatcher* );
    void promptFinished( KJob* );
    
public:
    CollectionPrivate            *collectionPrivate;
    ChangeCollectionPasswordJob  *theJob;
};

} // namespace

#endif // KSECRETSSERVICECOLLECTIONJOBS_P_H
