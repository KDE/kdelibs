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

#include <QObject>
#include <QDBusPendingReply>

class OrgFreedesktopSecretPromptInterface;

using namespace KSecretsService;

class CollectionJobPrivate {
public:
    CollectionJobPrivate();
    
    Collection      *collection;
    CollectionJob::CollectionError error;
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
    RenameCollectionJob *renameJob;
    CollectionPrivate   *collectionPrivate;
    QString             newName;
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