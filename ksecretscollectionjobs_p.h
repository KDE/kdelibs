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

#ifndef KSECRETSCOLLECTIONJOBS_P_H
#define KSECRETSCOLLECTIONJOBS_P_H

#include "ksecretscollectionjobs.h"
#include "../daemon/frontend/secret/adaptors/secretstruct.h"

#include <QObject>
#include <QDBusPendingReply>

namespace KSecretsService {
class SecretItem;
}

class OrgFreedesktopSecretPromptInterface;
class SecretPrivate;

using namespace KSecretsService;

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

    void startCreateCollection();
    void startOpenCollection();
    
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

class SearchItemsJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SearchItemsJobPrivate)
public:
    explicit SearchItemsJobPrivate( CollectionPrivate*, SearchItemsJob* );
    
    void startSearchItems();
    
Q_SIGNALS:
    void searchIsDone(CollectionJob::CollectionError, const QString& );

protected Q_SLOTS:
    void searchFinished(QDBusPendingCallWatcher*);
    
public:
    CollectionPrivate           *collectionPrivate;
    SearchItemsJob              *searchItemJob;
    QStringStringMap            attributes;
    QList< SecretItemPrivate >  items;
};

class CreateItemJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(CreateItemJobPrivate)
public:
    explicit CreateItemJobPrivate( CollectionPrivate*, QObject *parent =0 );

    void startCreateItem();
    
protected Q_SLOTS:
    void createItemReply(QDBusPendingCallWatcher*);
    void createPromptFinished(KJob*);
    
Q_SIGNALS:
    void createIsDone( CollectionJob::CollectionError, const QString& );
    
public:
    CollectionPrivate                   *collectionPrivate;
    CreateItemJob                       *createItemJob;
    QString                             label;
    QMap< QString, QString >            attributes;
    QSharedPointer< SecretPrivate >     secretPrivate;
    bool                                replace;
    SecretItem                          *item;
};

class SearchSecretsJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SearchSecretsJobPrivate)
public:
    explicit SearchSecretsJobPrivate( CollectionPrivate*, const QStringStringMap &attrs, QObject *parent =0);
    
    void startSearchSecrets();
    
protected Q_SLOTS:
    void searchSecretsReply(QDBusPendingCallWatcher*);
    void getSecretsReply(QDBusPendingCallWatcher*);
    
Q_SIGNALS:
    void searchIsDone( CollectionJob::CollectionError, const QString& );
    
public:
    CollectionPrivate   *collectionPrivate;
    QStringStringMap    attributes;
    QList< QSharedPointer<SecretPrivate> > secretsList;
};


class PromptJob : public KJob {
    Q_OBJECT
    Q_DISABLE_COPY(PromptJob)
public:
    PromptJob( const QDBusObjectPath &path, const WId &parentWindowId, QObject *parent );
    
    virtual void start();

    bool isDismissed() const { return dismissed; }
    const QDBusVariant & result() const { return opResult; }
    
private Q_SLOTS:
    void promptCompleted(bool dismissed, const QDBusVariant &result);
    
private:
    QDBusObjectPath promptPath;
    WId             parentWindowId;
    OrgFreedesktopSecretPromptInterface *promptIf;
    bool            dismissed;
    QDBusVariant    opResult;
};


#endif // KSECRETSCOLLECTIONJOBS_P_H