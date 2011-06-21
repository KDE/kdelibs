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

#include <QtDBus/QDBusConnection>
#include <kdebug.h>


#define SERVICE_NAME "org.freedesktop.secrets"

const QString DBusSession::encryptionAlgorithm = "dh-ietf1024-aes128-cbc-pkcs7";
OrgFreedesktopSecretServiceInterface *DBusSession::serviceIf = 0;
OrgFreedesktopSecretSessionInterface *DBusSession::sessionIf = 0;

bool DBusSession::startDaemon()
{
    // TODO: implement this
    // launch the daemon if it's not yet started
//     if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1( SERVICE_NAME )))
//     {
//         QString error;
//         // FIXME: find out why this is not working
//         int ret = KToolInvocation::startServiceByDesktopPath("ksecretsserviced.desktop", QStringList(), &error);
//         QVERIFY2( ret == 0, qPrintable( error ) );
//         
//         QVERIFY2( QDBusConnection::sessionBus().interface()->isServiceRegistered(QString::fromLatin1( SERVICE_NAME )),
//                  "Secret Service was started but the service is not registered on the DBus");
//     }

    return false;
}

OpenSessionJob* DBusSession::service()
{
    return new OpenSessionJob();
}

bool DBusSession::isValid()
{
    return sessionIf != 0 && sessionIf->isValid();
}

OpenSessionJob::OpenSessionJob(QObject* parent): 
            KJob(parent),
            sessionIf(0),
            serviceIf(0)
{
}

void OpenSessionJob::start()
{
    if ( DBusSession::isValid() ) {
        setError(0);
        emitResult();
    }
    else {
        serviceIf = new OrgFreedesktopSecretServiceInterface( SERVICE_NAME, 
                                                              "/org/freedesktop/secrets", 
                                                              QDBusConnection::sessionBus() );
        
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
}

OrgFreedesktopSecretServiceInterface* OpenSessionJob::serviceInterface() const
{
    // FIXME: should we check it or not?
    return DBusSession::serviceIf;
}
