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

#include "ksecretsserviceitemjobs.h"
#include "ksecretsserviceitemjobs_p.h"
#include "ksecretsserviceitem_p.h"
#include "ksecretsservicesecret_p.h"
#include "dbusbackend.h"

#include <item_interface.h>
#include <kdebug.h>
#include <QDBusPendingCallWatcher>

using namespace KSecretsService;


SecretItemJob::SecretItemJob(SecretItem* item): 
    secretItem( item )
{
}

void SecretItemJob::finished(SecretItemJob::ItemJobError err, const QString& msg)
{
    KJob::setError( err );
    KJob::setErrorText( msg );
    emitResult();
}




GetSecretItemSecretJob::GetSecretItemSecretJob( KSecretsService::SecretItem* item ) :
    SecretItemJob( item ),
    d( new GetSecretItemSecretJobPrivate( this ) )
{
    d->secretItemPrivate = item->d.data();
}


void GetSecretItemSecretJob::start()
{
    d->start();
}

Secret GetSecretItemSecretJob::secret() const
{
    return Secret( new SecretPrivate( d->secret ) );
}


GetSecretItemSecretJobPrivate::GetSecretItemSecretJobPrivate(GetSecretItemSecretJob * j) :
    job( j )
{
}

void GetSecretItemSecretJobPrivate::start()
{
    QDBusPendingReply<SecretStruct> reply = job->d->secretItemPrivate->itemIf->GetSecret( DBusSession::sessionPath() );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(getSecretReply(QDBusPendingCallWatcher*)) );
}

void GetSecretItemSecretJobPrivate::getSecretReply( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply<SecretStruct> reply = *watcher;
    if ( !reply.isError() ) {
        secret = reply.argumentAt<0>();
        job->finished( SecretItemJob::NoError );
    }
    else {
        kDebug() << "ERROR calling GetSecret";
        job->finished( SecretItemJob::InternalError, "ERROR calling GetSecret" );
    }
    watcher->deleteLater();
}


SetSecretItemSecretJob::SetSecretItemSecretJob( SecretItem* item, const Secret &s ) :
    SecretItemJob( item ),
    d( new SetSecretItemSecretJobPrivate( this, s ) )
{
    d->secretItemPrivate = item->d.data();
    d->secretPrivate = s.d.data();
}

void SetSecretItemSecretJob::start()
{
    d->start();
}

SetSecretItemSecretJobPrivate::SetSecretItemSecretJobPrivate( SetSecretItemSecretJob *j, const Secret &s ) :
    job( j ),
    secret( s )
{
}
    
void SetSecretItemSecretJobPrivate::start()
{
    SecretStruct secretStruct;
    if ( secretPrivate->toSecretStruct( secretStruct ) ) {
        QDBusPendingReply< void > reply = secretItemPrivate->itemIf->SetSecret( secretStruct );
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
        connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(setSecretReply(QDBusPendingCallWatcher*)) );
    }
    else {
        kDebug() << "ERROR building SecretStruct";
        job->finished( SecretItemJob::InternalError, "ERROR building SecretStruct" );
    }
}

void SetSecretItemSecretJobPrivate::setSecretReply( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply< void > reply = *watcher;
    if ( reply.isError() ) {
        kDebug() << "ERROR calling setSecret : " << reply.error().message();
        job->finished( SecretItemJob::InternalError, reply.error().message() );
    }
    else {
        job->finished( SecretItemJob::NoError );
    }
}


#include "ksecretsserviceitemjobs.moc"
#include "ksecretsserviceitemjobs_p.moc"
