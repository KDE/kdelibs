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
#include "promptjob.h"

using namespace KSecretsService;


SecretItemJob::SecretItemJob(SecretItem* item): 
    secretItem( item )
{
}

SecretItemJob::~SecretItemJob()
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

GetSecretItemSecretJob::~GetSecretItemSecretJob()
{
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

SetSecretItemSecretJob::~SetSecretItemSecretJob()
{
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


SecretItemDeleteJob::SecretItemDeleteJob( SecretItem * item, const WId &promptParentWindowId ) :
    SecretItemJob( item ),
    d( new SecretItemDeleteJobPrivate( this ) )
{
    d->secretItemPrivate = item->d.data();
    d->promptWinId = promptParentWindowId;
}

SecretItemDeleteJob::~SecretItemDeleteJob()
{
}

void SecretItemDeleteJob::start()
{
    d->startDelete();
}

SecretItemDeleteJobPrivate::SecretItemDeleteJobPrivate( SecretItemDeleteJob *j ) :
    job( j )
{
}
    
void SecretItemDeleteJobPrivate::startDelete()
{
    QDBusPendingReply< QDBusObjectPath > reply =  secretItemPrivate->itemIf->Delete();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(deleteItemReply(QDBusPendingCallWatcher*)) );
}

void SecretItemDeleteJobPrivate::deleteItemReply( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT( watcher );
    QDBusPendingReply< QDBusObjectPath > reply = *watcher;
    if ( ! reply.isError() ) {
        QDBusObjectPath promptPath = reply.argumentAt<0>();
        if ( promptPath.path().compare( "/" ) == 0 ) {
            job->finished( SecretItemJob::NoError );
        }
        else {
            PromptJob *promptjob = new PromptJob( promptPath, promptWinId, this );
            if ( job->addSubjob( promptjob ) ) {
                connect( promptjob, SIGNAL(finished(KJob*)), this, SLOT(deletePromptFinished(KJob*)) );
                promptjob->start();
            }
            else {
                promptjob->deleteLater();
                kDebug() << "Cannot add subjob for prompt " << promptPath.path();
                job->finished( SecretItemJob::InternalError, "Cannot start prompt job for the delete operation!" );
            }
        }
    }
    else {
        kDebug() << "ERROR when calling item.Delete() : " << reply.error().message();
        job->finished( SecretItemJob::InternalError, reply.error().message() );
    }
    watcher->deleteLater();
}

void SecretItemDeleteJobPrivate::deletePromptFinished( KJob* j)
{
    PromptJob *promptJob = dynamic_cast< PromptJob* >( j );
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            // TODO: should we read some result here or simply checking isDismissed is enough ?
            job->finished( SecretItemJob::NoError );
        }
        else {
            job->finished( SecretItemJob::OperationCancelledByTheUser, "The operation was cancelled by the user" );
        }
    }
    else {
        job->finished( SecretItemJob::InternalError, "Error encountered when trying to prompt the user" );
    }
}


ReadItemPropertyJob::ReadItemPropertyJob( SecretItem *item, const char *propName ) :
    SecretItemJob( item ),
    d( new ReadItemPropertyJobPrivate( item->d.data(), this ) ),
    propertyReadMember(0)
{
    d->propertyName = propName;
}

ReadItemPropertyJob::ReadItemPropertyJob( SecretItem *item, void (SecretItem::*propReadMember)( ReadItemPropertyJob* ) ) :
    SecretItemJob( item ),
    d( new ReadItemPropertyJobPrivate( item->d.data(), this ) ),
    propertyReadMember( propReadMember )
{
}

ReadItemPropertyJob::~ReadItemPropertyJob()
{
}

void ReadItemPropertyJob::start()
{
    if ( propertyReadMember ) {
        (secretItem->*propertyReadMember)( this );
        finished( NoError );
    }
    else {
        d->startReadingProperty();
    }
}

const QVariant& ReadItemPropertyJob::propertyValue() const
{
    return d->value;
}

ReadItemPropertyJobPrivate::ReadItemPropertyJobPrivate( SecretItemPrivate *it, ReadItemPropertyJob *job ) :
    itemPrivate( it ),
    readPropertyJob( job )
{
}
    
void ReadItemPropertyJobPrivate::startReadingProperty()
{
    value = itemPrivate->itemIf->property( propertyName );
    readPropertyJob->finished( SecretItemJob::NoError );
}


WriteItemPropertyJob::WriteItemPropertyJob( SecretItem *item, const char *propName, const QVariant& value ) :
    SecretItemJob( item ),
    d( new WriteItemPropertyJobPrivate( item->d.data(), this ) )
{
    d->propertyName = propName;
    d->value = value;
}

WriteItemPropertyJob::~WriteItemPropertyJob()
{
}

void WriteItemPropertyJob::start()
{
    d->startWritingProperty();
}

WriteItemPropertyJobPrivate::WriteItemPropertyJobPrivate( SecretItemPrivate *cp, WriteItemPropertyJob *job ) :
    itemPrivate( cp ),
    writePropertyJob( job )
{
}
    
void WriteItemPropertyJobPrivate::startWritingProperty()
{
    value = itemPrivate->itemIf->setProperty( propertyName, value );
    writePropertyJob->finished( SecretItemJob::NoError );
}



#include "ksecretsserviceitemjobs.moc"
#include "ksecretsserviceitemjobs_p.moc"
