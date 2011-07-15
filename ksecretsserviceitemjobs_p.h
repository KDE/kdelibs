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
    GetSecretItemSecretJobPrivate( KSecretsService::GetSecretItemSecretJob * );

    void start();
    
    SecretStruct secret;
    SecretItemPrivate *secretItemPrivate;
    
private Q_SLOTS:
    void getSecretReply( QDBusPendingCallWatcher* watcher );
    
Q_SIGNALS:
    void getSecretFinished( SecretItemJob::ItemJobError, const QString& );
    
private:
    GetSecretItemSecretJob *job;
};

#endif // KSECRETSSERVICEITEMJOBS_P_H
