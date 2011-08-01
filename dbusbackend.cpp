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
#include "item_interface.h"

#include <QtDBus/QDBusConnection>
#include <QtCrypto/QtCrypto>
#include <kdebug.h>
#include <klocalizedstring.h>


#define SERVICE_NAME "org.freedesktop.secrets"

const QString DBusSession::encryptionAlgorithm = "dh-ietf1024-aes128-cbc-pkcs7";
OpenSessionJob DBusSession::openSessionJob(0);
DBusSession DBusSession::staticInstance;

DBusSession::DBusSession()
{
    openSessionJob.setAutoDelete(false);
}

OpenSessionJob* DBusSession::openSession()
{
    return &openSessionJob;
}

OpenSessionJob::OpenSessionJob(QObject* parent): 
            KJob(parent),
            sessionIf(0),
            serviceIf(0),
            dhPrivkey(0)
{
}

OpenSessionJob::~OpenSessionJob()
{
    delete dhPrivkey;
}

void OpenSessionJob::start()
{
    if ( serviceIf && serviceIf->isValid() && sessionIf && sessionIf->isValid() ) {
        setError(0);
        emitResult();
    }
    else {
        qRegisterMetaType<SecretStruct>();
        qRegisterMetaType<StringStringMap>();
        
        qDBusRegisterMetaType<SecretStruct>();
        qDBusRegisterMetaType<StringStringMap>();
        qDBusRegisterMetaType<ObjectPathSecretMap>();
        qDBusRegisterMetaType<StringVariantMap>();
        
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
            

            QCA::KeyGenerator keygen;
            dhDlgroup = new QCA::DLGroup(keygen.createDLGroup(QCA::IETF_1024));
            if ( dhDlgroup->isNull() ) {
                QString errorTxt = i18n("Cannot create DL Group for dbus session open");
                kDebug() << errorTxt;
                setError(1); // FIXME: use error codes here
                setErrorText( errorTxt );
                emitResult();
            }
            else {
                dhPrivkey = new QCA::PrivateKey(keygen.createDH(*dhDlgroup));
                QCA::PublicKey dhPubkey(*dhPrivkey);
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
            kDebug() << "ERROR when trying to bind to " SERVICE_NAME " daemon";
            setError( 3 ); // FIXME: use error codes here
            setErrorText( ki18n( "ERROR when trying to bind to %1 daemon. Check dbus configuration." ).subs(SERVICE_NAME).toString() );
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
        QVariant dhOutputVar = reply.argumentAt<0>().variant();
        QByteArray dhOutput = dhOutputVar.toByteArray();
        QCA::DHPublicKey dhServiceKey(*dhDlgroup, QCA::BigInteger(QCA::SecureArray(dhOutput)));
        QCA::SymmetricKey dhSharedKey(dhPrivkey->deriveKey(dhServiceKey));
        QCA::Cipher *dhCipher = new QCA::Cipher("aes128", QCA::Cipher::CBC, QCA::Cipher::PKCS7);
        
        QDBusObjectPath sessionPath = reply.argumentAt<1>();
        kDebug() << "SESSION path is " << sessionPath.path();
        sessionIf = new OrgFreedesktopSecretSessionInterface( SERVICE_NAME, sessionPath.path(), QDBusConnection::sessionBus() );
        setError(0);
        setErrorText( i18n("OK") );
        emitResult();
    }
    watcher->deleteLater();
}

OrgFreedesktopSecretServiceInterface* OpenSessionJob::serviceInterface() const
{
    Q_ASSERT( serviceIf != 0 ); // you should call openSession first and start the job it returns before calling this method
    return serviceIf;
}

OrgFreedesktopSecretSessionInterface* OpenSessionJob::sessionInterface() const
{
    Q_ASSERT( sessionIf != 0 ); // you should call openSession first and start the job it returns before calling this method
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

OrgFreedesktopSecretItemInterface* DBusSession::createItem(const QDBusObjectPath& path)
{
    return new OrgFreedesktopSecretItemInterface( SERVICE_NAME, path.path(), QDBusConnection::sessionBus() );
}

QDBusObjectPath DBusSession::sessionPath()
{
    Q_ASSERT( openSessionJob.sessionInterface()->isValid() );
    return QDBusObjectPath( openSessionJob.sessionInterface()->path() );
}

bool DBusSession::encrypt(const QVariant& value, SecretStruct& secretStruct)
{
    QCA::SecureArray valueArray( value.toByteArray() );
    QCA::SecureArray encryptedArray;
    bool result = openSessionJob.secretCodec.encryptClient( valueArray , encryptedArray, secretStruct.m_parameters );
    if ( result ) {
        secretStruct.m_value = encryptedArray.toByteArray();
    }
    return result;
}

bool DBusSession::decrypt(const SecretStruct& secretStruct, QVariant& value)
{
    QCA::SecureArray valueArray;
    bool result = openSessionJob.secretCodec.decryptClient( secretStruct.m_value, secretStruct.m_parameters, valueArray );
    if ( result ) {
        value = valueArray.toByteArray();
    }
    return result;
}

OrgFreedesktopSecretServiceInterface* DBusSession::serviceIf()
{
    return openSessionJob.serviceIf;
}
