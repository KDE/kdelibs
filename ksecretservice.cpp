/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Dario Freddi <drf@kde.org>
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


#include "ksecretservice.h"
#include "ksecretservice_p.h"

#include "ksecretservicecollection.h"

// Types
#include "../daemon/frontend/secret/adaptors/dbustypes.h"

// DBus interfaces
#include "collection_interface.h"
#include "prompt_interface.h"
#include "service_interface.h"
#include "session_interface.h"

#include <kglobal.h>
#include <klocalizedstring.h>
#include <kdebug.h>


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
        // FIXME: i'd prefer the static initilization be here instead in constructor code
        Q_ASSERT( s_globalKSecretService->q != NULL );
    }

    return s_globalKSecretService->q;
}

// Jobs

KSecretService::CreateCollectionJob::CreateCollectionJob(const QString& label, const QVariantMap& properties, KSecretService *parent)
    : KJob(parent)
    , m_kss(parent)
    , m_label(label)
    , m_properties(properties)
{
}

KSecretService::CreateCollectionJob::~CreateCollectionJob() {
    
}

void KSecretService::CreateCollectionJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void KSecretService::CreateCollectionJob::run()
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

void KSecretService::CreateCollectionJob::onReplyFinished(QDBusPendingCallWatcher *reply)
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

KSecretService::DeleteCollectionJob::DeleteCollectionJob(const QString& label, KSecretService *parent)
    : KJob(parent)
    , m_kss(parent)
    , m_label(label)
{
}

KSecretService::DeleteCollectionJob::~DeleteCollectionJob() {
}

void KSecretService::DeleteCollectionJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void KSecretService::DeleteCollectionJob::run()
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

void KSecretService::DeleteCollectionJob::onReplyFinished(QDBusPendingCallWatcher *reply)
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
    , _sessionAlgorithm( ALGORITHM_PLAIN )
{
    Q_ASSERT(!s_globalKSecretService->q);
    s_globalKSecretService->q = this;
}

KSecretService::~KSecretService()
{
    delete d;
}

void KSecretService::setAlgorithm(KSecretService::SessionEncryptingAlgorithm sessionAlgorithm)
{
    Q_ASSERT( !isConnected() );
    _sessionAlgorithm = sessionAlgorithm;
}

void KSecretService::connectToService()
{
    if ( isConnected() )
        return;
    
    // Create a session and make everything ready
    QCA::init();
    registerDBusTypes();

    // Create the service interface!
    d->serviceInterface = new OrgFreedesktopSecretServiceInterface("org.freedesktop.Secret",
                                                                   "/org/freedesktop/secrets",
                                                                   QDBusConnection::sessionBus(),
                                                                   this);

    const char *algorithm = 0;
    switch ( _sessionAlgorithm ) {
        case ALGORITHM_PLAIN:
            algorithm = "plain";
            break;
            // TODO: add other algorithms here
        default:
            kDebug() << "Unknown session algorithm " << _sessionAlgorithm;
            Q_ASSERT(0);
            return;
    }
    
//     QDBusPendingReply< QDBusVariant, QDBusObjectPath > reply = d->serviceInterface->OpenSession(algorithm, QDBusVariant(""));
//     reply.waitForFinished();
//
//     if (reply.isError()) {
//         kDebug() << "Cannot open KSecretService session";
//         return;
//     }
//
//     QDBusObjectPath sessionPath = reply.reply().arguments().last().value< QDBusObjectPath >();

    QDBusObjectPath sessionPath;
    QDBusReply< QDBusVariant > reply = d->serviceInterface->OpenSession( algorithm, QDBusVariant(""), sessionPath );


    if (sessionPath.path().isEmpty()) {
        kDebug() << "KSecretService session path is empty!";
        return;
    }

    // Create the session interface
    d->sessionInterface = new OrgFreedesktopSecretSessionInterface("org.freedesktop.Secret",
                                                                   sessionPath.path(),
                                                                   QDBusConnection::sessionBus(),
                                                                   this);
    if ( !d->sessionInterface->isValid() ) {
        kDebug() << "Cannot open KSecretService session interface";
    }
}

bool KSecretService::isConnected() const
{
    return d->serviceInterface != NULL && d->serviceInterface->isValid() && d->sessionInterface->isValid();
}

OrgFreedesktopSecretServiceInterface* KSecretService::service() 
{
    if ( !d->serviceInterface || !d->serviceInterface->isValid() ) {
        connectToService();
    }
    return d->serviceInterface;
}

OrgFreedesktopSecretSessionInterface* KSecretService::session() 
{
    if ( !isConnected() ) {
        connectToService();
    }
    return d->sessionInterface;
}

OrgFreedesktopSecretServiceInterface* KSecretService::service() 
{
    if ( !isConnected() ) {
        connectToService();
    }
    return d->serviceInterface;
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

KJob *KSecretService::createCollectionAsync(const QString& label, const QVariantMap& properties)
{
    return new CreateCollectionJob(label, properties, this);
}

KSecretServiceCollection * KSecretService::createCollection(const QString& label, const QVariantMap& properties)
{
    KSecretServiceCollection * result = 0;
    QVariantMap collectionProps = properties;
    collectionProps.insert("Label", label);
    QDBusObjectPath promptPath;
    QDBusReply<QDBusObjectPath> collPathReply = service()->CreateCollection( collectionProps, promptPath );
    QString collectionPath = collPathReply.value().path();
    if ( collectionPath == "/" ) {
        collectionPath.clear();
        OrgFreedesktopSecretPromptInterface promptIf( "org.freedesktop.Secret", promptPath.path(), d->serviceInterface->connection() );
        if ( promptIf.isValid() ) {
            d->connect( &promptIf, SIGNAL( Completed(bool, const QDBusVariant&) ), SLOT( createCollectionPromptCompleted(bool, const QDBusVariant&)));
            QDBusPendingReply<> promptReply = promptIf.Prompt("");
            promptReply.waitForFinished();
            // NOTE waitForFinished above does not mean that collection was created
            // NOTE the prompt launch creation on a separate job and we must wait
            // NOTE for the createCollectionPromptCompleted slot to be called before continuing
            if ( !promptReply.isError() ) {
                d->createCollectionDone = false;
                do {
                    d->localEventLoop.processEvents();
                } while ( !d->createCollectionDone );
                
                if ( !d->createCollectionDismissed )
                    collectionPath = d->createCollectionPath;
            }
        }
        else {
            kDebug() << "Cannot get prompt interface from path " << promptPath.path();
        }
    }
    if ( !collectionPath.isEmpty() ) {
        result = new KSecretServiceCollection( collectionPath, this );
    }
    return result;
}

void KSecretService::Private::createCollectionPromptCompleted(bool dismissed, const QDBusVariant& collectionPath)
{
    createCollectionDismissed = dismissed;
    if ( createCollectionDismissed ) {
        createCollectionPath.clear();
    }
    else {
        createCollectionPath = collectionPath.variant().value<QDBusObjectPath>().path();
    }
    createCollectionDone = true;
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

