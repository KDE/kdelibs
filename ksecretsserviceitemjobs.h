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

#ifndef KSECRETSSERVICEITEMJOBS_H
#define KSECRETSSERVICEITEMJOBS_H

#include "ksecretsservicesecret.h"

#include <QSharedPointer>
#include <kcompositejob.h>
#include <qwindowdefs.h>

class GetSecretItemSecretJobPrivate;
class SetSecretItemSecretJobPrivate;
class SecretItemDeleteJobPrivate;

namespace KSecretsService {

class SecretItem;

class SecretItemJob : public KCompositeJob {
    Q_OBJECT
    Q_DISABLE_COPY(SecretItemJob)
public:
    enum ItemJobError {
        UndefinedError =-1,             /// this error should never be encountered
        NoError =0,
        InternalError,
        OperationCancelledByTheUser,    /// the user choose to cancel ther operation during a message prompt
        CollectionNotFound,
        CreateError,
        DeleteError,
        RenameError,
        MissingParameterError
    };
    
    explicit SecretItemJob( SecretItem * item );
    
protected:
    void finished( ItemJobError, const QString& msg ="");
    
public:
    SecretItem  *secretItem;
};

class GetSecretItemSecretJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(GetSecretItemSecretJob)
public:
    explicit GetSecretItemSecretJob( SecretItem* );
    virtual void start();
    Secret secret() const;
    
private:
    friend class ::GetSecretItemSecretJobPrivate;
    QSharedPointer< GetSecretItemSecretJobPrivate > d;
};

class SetSecretItemSecretJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(SetSecretItemSecretJob)
public:
    SetSecretItemSecretJob( SecretItem*, const Secret& );
    
    virtual void start();
    
private:
    friend class ::SetSecretItemSecretJobPrivate;
    QSharedPointer< SetSecretItemSecretJobPrivate > d;
};

class SecretItemDeleteJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(SecretItemDeleteJob)
public:
    SecretItemDeleteJob( SecretItem*, const WId &promptParentWindowId );
    
    virtual void start();
    
private:
    friend class ::SecretItemDeleteJobPrivate;
    QSharedPointer< SecretItemDeleteJobPrivate > d;
};

}; // namespace

#endif // KSECRETSSERVICEITEMJOBS_H
