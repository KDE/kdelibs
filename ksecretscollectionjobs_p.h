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

using namespace KSecretsService;

/**
 * Internal DeleteCollectionJob implementation, based on DBus
 */
class DeleteCollectionJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(DeleteCollectionJobPrivate);
public:
    explicit DeleteCollectionJobPrivate( QObject *parent =0 );
    
    friend class DeleteCollectionJob;
    
    QDBusPendingReply<QDBusObjectPath> deleteReply;
    
Q_SIGNALS:
    void deleteIsDone( CollectionJob::CollectionError error, const QString& message );
    
protected Q_SLOTS:
    void callFinished( QDBusPendingCallWatcher* );
};

class FindCollectionJobPrivate  {
    
public:
    friend class FindCollectionJob;
    
    FindCollectionJobPrivate();
    FindCollectionJobPrivate( QString collName, KSecretsService::Collection::FindCollectionOptions opts );
    
    QString                                              collectionName;
    KSecretsService::Collection::FindCollectionOptions   findCollectionOptions;
};
    
#endif // KSECRETSCOLLECTIONJOBS_P_H