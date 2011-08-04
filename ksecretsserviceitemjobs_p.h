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

#ifndef KSECRETSSERVICEITEMJOBS_P_H
#define KSECRETSSERVICEITEMJOBS_P_H

#include "ksecretsserviceitemjobs.h"
#include "../daemon/frontend/secret/adaptors/secretstruct.h"
#include <qdbusconnection.h>
#include <qwindowdefs.h>

namespace KSecretsService {
    class GetSecretItemSecretJob;
};
using namespace KSecretsService;

class QDBusPendingCallWatcher;
class SecretItemPrivate;

class GetSecretItemSecretJobPrivate  : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(GetSecretItemSecretJobPrivate)
public:
    GetSecretItemSecretJobPrivate( GetSecretItemSecretJob * );

    void start();
    
private Q_SLOTS:
    void getSecretReply( QDBusPendingCallWatcher* watcher );
    
Q_SIGNALS:
    void getSecretFinished( SecretItemJob::ItemJobError, const QString& );

public:
    GetSecretItemSecretJob *job;
    SecretStruct secret;
    SecretItemPrivate *secretItemPrivate;
};

class SetSecretItemSecretJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SetSecretItemSecretJobPrivate)
public:
    SetSecretItemSecretJobPrivate( SetSecretItemSecretJob*, const Secret & );
    
    void start();
    
private Q_SLOTS:
    void setSecretReply( QDBusPendingCallWatcher* watcher );

public:
    SetSecretItemSecretJob *job;
    Secret secret;
    SecretItemPrivate *secretItemPrivate;
    const SecretPrivate *secretPrivate;
};

class SecretItemDeleteJobPrivate : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(SecretItemDeleteJobPrivate)
public:
    SecretItemDeleteJobPrivate( SecretItemDeleteJob* );
    
    void startDelete();
    
private Q_SLOTS:
    void deleteItemReply( QDBusPendingCallWatcher *watcher );
    void deletePromptFinished(KJob*);
    
public:
    SecretItemDeleteJob *job;
    SecretItemPrivate   *secretItemPrivate;
    WId                 promptWinId;
};

class ReadItemPropertyJobPrivate : public QObject {
public:
    explicit ReadItemPropertyJobPrivate( SecretItemPrivate*, ReadItemPropertyJob* );
    
    void startReadingProperty();
    
    SecretItemPrivate *itemPrivate;
    ReadItemPropertyJob *readPropertyJob;
    const char *propertyName;
    QVariant value;
};

class WriteItemPropertyJobPrivate : public QObject {
public:
    explicit WriteItemPropertyJobPrivate( SecretItemPrivate*, WriteItemPropertyJob* );
    
    void startWritingProperty();
    
    SecretItemPrivate *itemPrivate;
    WriteItemPropertyJob *writePropertyJob;
    const char *propertyName;
    QVariant value;
};


#endif // KSECRETSSERVICEITEMJOBS_P_H
