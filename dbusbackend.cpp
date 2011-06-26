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

#include "dbusbackend.h"
#include "service_interface.h"
#include "collection_interface.h"
#include "session_interface.h"
#include "prompt_interface.h"

#include <QtDBus/QDBusConnection>
#include <kdebug.h>


#define SERVICE_NAME "org.freedesktop.secrets"

const QString DBusSession::encryptionAlgorithm = "dh-ietf1024-aes128-cbc-pkcs7";
OpenSessionJob DBusSession::openSessionJob(0);


OpenSessionJob* DBusSession::openSession()
{
    return new OpenSessionJob();
}

OpenSessionJob::OpenSessionJob(QObject* parent): 
            KJob(parent),
            sessionIf(0),
            serviceIf(0)
{
}

void OpenSessionJob::start()
{
    if ( serviceIf && serviceIf->isValid() && sessionIf && sessionIf->isValid() ) {
        setError(0);
        emitResult();
    }
    else {
        serviceIf = new OrgFreedesktopSecretServiceInterface( SERVICE_NAME, 
                                                              "/org/freedesktop/secrets", 
                                                              QDBusConnection::sessionBus() );
        
        if ( serviceIf->isValid() ) {
            QDBusConnectionInterface *serviceInfo = QDBusConnection::sessionBus().interface();
            QDBusReply< QString > ownerReply = serviceInfo->serviceOwner( SERVICE_NAME );
            QDBusReply< uint > pidReply = serviceInfo->servicePid( SERVICE_NAME );
            if ( ownerReply.isValid() && pidReply.isValid() ) {
                kDebug() << "SERVICE owner is " << (QString)ownerReply << ", PID = " << (uint)pidReply;
            }
            else {
                kDebug() << "Cannot get SERVICE information";
            }
            
            static bool qcaInitialized = false;
            if ( !qcaInitialized ) {
                QCA::init();
                qcaInitialized = true;
            }
            
            QCA::KeyGenerator keygen;
            QCA::DLGroup dhDlgroup(keygen.createDLGroup(QCA::IETF_1024));
            if ( dhDlgroup.isNull() ) {
                QString errorTxt = "Cannot create DL Group for dbus session open";
                kDebug() << errorTxt;
                setError(1); // FIXME: use error codes here
                setErrorText( errorTxt );
                emitResult();
            }
            else {
                QCA::PrivateKey dhPrivkey(keygen.createDH(dhDlgroup));
                QCA::PublicKey dhPubkey(dhPrivkey);
                QByteArray dhBytePub(dhPubkey.toDH().y().toArray().toByteArray());
                
                QDBusPendingReply< QDBusVariant, QDBusObjectPath > openSessionReply = serviceIf->OpenSession( 
                    DBusSession::encryptionAlgorithm,
                    QDBusVariant(dhBytePub)
                );
                QDBusPendingCallWatcher *openSessionWatcher = new QDBusPendingCallWatcher( openSessionReply, this );
                connect( openSessionWatcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(openSessionFinished(QDBusPendingCallWatcher*)) );
            }
        }
        else {
            kDebug() << "ERROR wheng trying to bind to " SERVICE_NAME " daemon";
            setError( 3 ); // FIXME: use error codes here
            setErrorText( "ERROR wheng trying to bind to " SERVICE_NAME " daemon" );
            emitResult();
        }
    }
}

void OpenSessionJob::openSessionFinished(QDBusPendingCallWatcher* watcher)
{
    Q_ASSERT( watcher->isFinished() );
    QDBusPendingReply< QDBusVariant, QDBusObjectPath > reply = *watcher;
    if ( watcher->isError() ) {
        kDebug() << "ERROR when attempting to open a session " << reply.error().message();
        setError(2); // FIXME: use error codes here
        setErrorText( reply.error().message() );
        emitResult();
    }
    else {
        QDBusObjectPath sessionPath = reply.argumentAt<1>();
        kDebug() << "SESSION path is " << sessionPath.path();
        sessionIf = new OrgFreedesktopSecretSessionInterface( SERVICE_NAME, sessionPath.path(), QDBusConnection::sessionBus() );
        setError(0);
        setErrorText("OK");
        emitResult();
    }
}

OrgFreedesktopSecretServiceInterface* OpenSessionJob::serviceInterface() const
{
    Q_ASSERT( serviceIf != 0 ); // you should call openSession first and start the job it returns before calling this method
    return serviceIf;
}

OrgFreedesktopSecretSessionInterface* OpenSessionJob::sessionInterface() const
{
    Q_ASSERT( serviceIf != 0 ); // you should call openSession first and start the job it returns before calling this method
    return sessionIf;
}

OrgFreedesktopSecretPromptInterface* DBusSession::createPrompt(const QDBusObjectPath& path)
{
    return new OrgFreedesktopSecretPromptInterface( SERVICE_NAME, path.path(), QDBusConnection::sessionBus() );
}

OrgFreedesktopSecretCollectionInterface* DBusSession::createCollection(const QDBusObjectPath& path)
{
    return new OrgFreedesktopSecretCollectionInterface( SERVICE_NAME, path.path(), QDBusConnection::sessionBus() );
}
