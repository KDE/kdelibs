/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Dario Freddi <drf@kde.org>
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


#include "ksecretservice.h"
#include "ksecretservice_p.h"

#include "ksecretservicecollection.h"

// Types
#include <adaptors/dbustypes.h>

// DBus interfaces
#include "collection_interface.h"
#include "prompt_interface.h"
#include "service_interface.h"
#include "session_interface.h"

#include <kglobal.h>
#include <klocalizedstring.h>


class KSecretServiceHelper
{
public:
    KSecretServiceHelper() : q(0) {}
    ~KSecretServiceHelper() {
        delete q;
    }
    KSecretService *q;
};

K_GLOBAL_STATIC(KSecretServiceHelper, s_globalKSecretService)

KSecretService *KSecretService::instance()
{
    if (!s_globalKSecretService->q) {
        new KSecretService;
    }

    return s_globalKSecretService->q;
}

// Jobs

CreateCollectionJob::CreateCollectionJob(const QString& label, const QVariantMap& properties, KSecretService *parent)
    : KJob(parent)
    , m_kss(parent)
    , m_label(label)
    , m_properties(properties)
{
}

void CreateCollectionJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void CreateCollectionJob::run()
{
    if (!m_properties.contains("Locked")) {
        m_properties["Locked"] = false; // create collection unlocked
    }

    m_properties["Label"] = m_label;

    QDBusPendingReply< QDBusVariant, QDBusObjectPath > reply = m_kss->d->serviceInterface->CreateCollection(m_properties);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onReplyFinished(QDBusPendingCallWatcher*)));
}

void CreateCollectionJob::onReplyFinished(QDBusPendingCallWatcher *reply)
{
    if (reply->isError()) {
        // TODO: Figure out what to do with errors
        setErrorText(i18n("DBus call failed - could not contact KSecretService"));
        emitResult();
        return;
    }

    QDBusObjectPath sessionPath = reply->reply().arguments().last().value< QDBusObjectPath >();

    if (sessionPath.path() == "/") {
        // TODO: Handle prompt
    }

    // Done
    emitResult();
}

////////////////////////////////

DeleteCollectionJob::DeleteCollectionJob(const QString& label, KSecretService *parent)
    : KJob(parent)
    , m_kss(parent)
    , m_label(label)
{
}

void DeleteCollectionJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void DeleteCollectionJob::run()
{
    QString retpath;
    foreach (const QDBusObjectPath &path, m_kss->d->serviceInterface->collections()) {
        OrgFreedesktopSecretCollectionInterface collection("org.freedesktop.Secret",
                                                           path.path(),
                                                           QDBusConnection::sessionBus());
        if (collection.isValid()) {
            if (collection.label() == m_label) {
                QDBusPendingReply< QDBusObjectPath > reply = collection.Delete();

                QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

                connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                        this, SLOT(onReplyFinished(QDBusPendingCallWatcher*)));
                return;
            }
        }
    }

    // If we got here, no matching collections were found
    setErrorText(i18n("No collections named %1 were found, aborting.", m_label));
    emitResult();
}

void DeleteCollectionJob::onReplyFinished(QDBusPendingCallWatcher *reply)
{
    if (reply->isError()) {
        setErrorText(i18n("DBus call failed - could not contact KSecretService"));
        emitResult();
        return;
    }

    if (reply->reply().arguments().first().value< QDBusObjectPath >().path() != "/") {
        // TODO Handle prompt
    }

    // Done
    emitResult();
}

KSecretService::KSecretService(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    Q_ASSERT(!s_globalKSecretService->q);
    s_globalKSecretService->q = this;
}

KSecretService::~KSecretService()
{
    delete d;
}

void KSecretService::init()
{
    // Create a session and make everything ready
    QCA::init();
    registerDBusTypes();

    // Create the service interface!
    d->serviceInterface = new OrgFreedesktopSecretServiceInterface("org.freedesktop.Secret",
                                                                   "/org/freedesktop/Secrets",
                                                                   QDBusConnection::sessionBus(),
                                                                   this);

    QDBusPendingReply< QDBusVariant, QDBusObjectPath > reply = d->serviceInterface->OpenSession("RSA", QDBusVariant());
    reply.waitForFinished();

    if (reply.isError()) {
        // Hmm, do something here
        return;
    }

    QDBusObjectPath sessionPath = reply.reply().arguments().last().value< QDBusObjectPath >();

    if (sessionPath.path().isEmpty()) {
        // Hmm, do something here
        return;
    }

    // Create the session interface
    d->sessionInterface = new OrgFreedesktopSecretSessionInterface("org.freedesktop.Secret",
                                                                   sessionPath.path(),
                                                                   QDBusConnection::sessionBus(),
                                                                   this);
}

QStringList KSecretService::collections() const
{
    // Obtain the collections labels
    QStringList collectionLabels;
    foreach (const QDBusObjectPath &path, d->serviceInterface->collections()) {
        OrgFreedesktopSecretCollectionInterface collection("org.freedesktop.Secret",
                                                           path.path(),
                                                           QDBusConnection::sessionBus());
        if (collection.isValid()) {
            collectionLabels << collection.label();
        }
    }
    return collectionLabels;
}

KJob *KSecretService::createCollection(const QString& label, const QVariantMap& properties)
{
    return new CreateCollectionJob(label, properties, this);
}

KSecretServiceCollection* KSecretService::openCollection(const QString& label)
{
    QString retpath;
    foreach (const QDBusObjectPath &path, d->serviceInterface->collections()) {
        OrgFreedesktopSecretCollectionInterface collection("org.freedesktop.Secret",
                                                           path.path(),
                                                           QDBusConnection::sessionBus());
        if (collection.isValid()) {
            if (collection.label() == label) {
                retpath = path.path();
                break;
            }
        }
    }
    return new KSecretServiceCollection(retpath, this);
}

KJob *KSecretService::deleteCollection(const QString& label)
{
    return new DeleteCollectionJob(label, this);
}

#include "ksecretservice.moc"
#include "ksecretservice_p.moc"
