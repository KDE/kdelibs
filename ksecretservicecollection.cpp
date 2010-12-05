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

#include "ksecretservicecollection.h"
#include "ksecretservicecollection_p.h"

#include "ksecretservice.h"
#include "ksecretservice_p.h"

// Types
#include <adaptors/dbustypes.h>

// DBus interfaces
#include "collection_interface.h"
#include "item_interface.h"
#include "prompt_interface.h"
#include "service_interface.h"
#include "session_interface.h"

#include <KLocalizedString>

class KSecretServiceCollection::Private {
public:
    KSecretService *kss;
    OrgFreedesktopSecretCollectionInterface *collectionInterface;
};

//// Jobs

class ReadEntryJob::Private
{
public:
    Private(const QString &label, KSecretServiceCollection *kssc, ReadEntryJob *parent)
        : kssc(kssc)
        , label(label)
        , parent(parent) {}

    KSecretServiceCollection *kssc;
    QString label;
    ReadEntryJob *parent;

    KSecretServiceCollection::Entry entry;

    // Q_PRIVATE_SLOT
    void run();
    void onSearchFinished(KJob *job);
};

ReadEntryJob::ReadEntryJob(const QString &label, KSecretServiceCollection* parent)
    : KJob(parent)
    , d(new Private(label, parent, this))
{
}

ReadEntryJob::~ReadEntryJob()
{
    delete d;
}

KSecretServiceCollection::Entry ReadEntryJob::entry() const
{
    return d->entry;
}

void ReadEntryJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void ReadEntryJob::Private::run()
{
    // First of all, let's search for the item.
    KJob *search = new SearchEntriesJob(label, QMap< QString, QString >(), kssc);
    parent->connect(search, SIGNAL(finished(KJob*)),
                    parent, SLOT(onSearchFinished(KJob*)));
}

void ReadEntryJob::Private::onSearchFinished(KJob* job)
{
    if (job->error() != 0) {
        // Could not search for the item, let's stack the errors
        parent->setErrorText(job->errorText());
        parent->emitResult();
        return;
    }

    SearchEntriesJob *realJob = qobject_cast< SearchEntriesJob* >(job);

    if (!realJob) {
        parent->setErrorText(i18n("Internal failure, got a different job type than expected."));
        parent->emitResult();
        return;
    }

    // Now let's have a look at the results
    if (realJob->result().isEmpty()) {
        parent->setErrorText(i18n("No items labeled %1 were found, aborting.", label));
        parent->emitResult();
        return;
    }

    // Everything went fine, let's read the item
    entry = KSecretServiceCollection::Entry(realJob->result().first(), kssc->d->kss->d->sessionInterface->path());

    // Done
    parent->emitResult();
}

////////////////////////////////

WriteEntryJob::WriteEntryJob(const QString &label, const QByteArray &secret,
                             const QVariantMap &attributes, KSecretServiceCollection *parent)
    : KJob(parent)
    , m_kssc(parent)
    , m_label(label)
    , m_secret(secret)
    , m_attributes(attributes)
{
}

WriteEntryJob::~WriteEntryJob()
{
}

void WriteEntryJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void WriteEntryJob::run()
{
    Secret secret;
    secret.setSession(QDBusObjectPath(m_kssc->d->kss->d->sessionInterface->path()));
    secret.setValue(m_secret);

    m_attributes["Label"] = m_label;

    QDBusPendingCall reply = m_kssc->d->collectionInterface->CreateItem(m_attributes, secret, true);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onReplyFinished(QDBusPendingCallWatcher*)));
}

void WriteEntryJob::onReplyFinished(QDBusPendingCallWatcher *reply)
{
    if (reply->isError()) {
        setErrorText(i18n("DBus call failed - could not contact KSecretService"));
        emitResult();
        return;
    }

    QDBusObjectPath promptPath = reply->reply().arguments().last().value< QDBusObjectPath >();

    if (promptPath.path() != "/") {
        // TODO: Handle prompt
    }

    // Done
    emitResult();
}

///////////////

DeleteEntryJob::DeleteEntryJob(const QString &label, KSecretServiceCollection* parent)
    : KJob(parent)
    , m_kssc(parent)
    , m_label(label)
{
}

DeleteEntryJob::~DeleteEntryJob()
{
}

void DeleteEntryJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void DeleteEntryJob::run()
{
    // First of all, let's search for the item.
    KJob *search = new SearchEntriesJob(m_label, QMap< QString, QString >(), m_kssc);
    connect(search, SIGNAL(finished(KJob*)),
            this, SLOT(onSearchFinished(KJob*)));
}

void DeleteEntryJob::onSearchFinished(KJob* job)
{
    if (job->error() != 0) {
        // Could not search for the item, let's stack the errors
        setErrorText(job->errorText());
        emitResult();
        return;
    }

    SearchEntriesJob *realJob = qobject_cast< SearchEntriesJob* >(job);

    if (!realJob) {
        setErrorText(i18n("Internal failure, got a different job type than expected."));
        emitResult();
        return;
    }

    // Now let's have a look at the results
    if (realJob->result().isEmpty()) {
        setErrorText(i18n("No items labeled %1 were found, aborting.", m_label));
        emitResult();
        return;
    }

    // Everything went fine, let's delete the item
    OrgFreedesktopSecretItemInterface interface("org.freedesktop.Secret",
                                                realJob->result().first(),
                                                QDBusConnection::sessionBus());
    QDBusPendingCall reply = interface.Delete();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onReplyFinished(QDBusPendingCallWatcher*)));
}

void DeleteEntryJob::onReplyFinished(QDBusPendingCallWatcher *reply)
{
    if (reply->isError()) {
        setErrorText(i18n("DBus call failed - could not contact KSecretService"));
        emitResult();
        return;
    }

    QDBusObjectPath promptPath = reply->reply().arguments().first().value< QDBusObjectPath >();

    if (promptPath.path() != "/") {
        // TODO: Handle prompt
    }

    // Done
    emitResult();
}

///////////////

SearchEntriesJob::SearchEntriesJob(const QString &label, const QMap< QString, QString > &properties,
                                   KSecretServiceCollection *parent)
    : KJob(parent)
    , m_kssc(parent)
    , m_label(label)
    , m_properties(properties)
{
}

SearchEntriesJob::~SearchEntriesJob()
{
}

void SearchEntriesJob::start()
{
    QTimer::singleShot(0, this, SLOT(run()));
}

void SearchEntriesJob::run()
{
    m_properties["Label"] = m_label;

    QDBusPendingCall reply = m_kssc->d->collectionInterface->SearchItems(m_properties);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(onReplyFinished(QDBusPendingCallWatcher*)));
}

void SearchEntriesJob::onReplyFinished(QDBusPendingCallWatcher *reply)
{
    if (reply->isError()) {
        setErrorText(i18n("DBus call failed - could not contact KSecretService"));
        emitResult();
        return;
    }

    QList< QDBusObjectPath > list = reply->reply().arguments().first().value< QList< QDBusObjectPath > >();
    foreach (const QDBusObjectPath &path, list) {
        m_result << path.path();
    }

    // Done
    emitResult();
}

//// Entry

class KSecretServiceCollection::Entry::Data : public QSharedData
{
public:
    Data() {}
    Data(const Data &other)
        : QSharedData(other)
        , attributes(other.attributes)
        , label(other.label)
        , created(other.created)
        , lastModified(other.lastModified)
        , secret(other.secret) {}
    ~Data() { }

    QMap< QString, QString > attributes;
    QString label;

    QDateTime created;
    QDateTime lastModified;

    Secret secret;
};

KSecretServiceCollection::Entry::Entry()
    : d(new Data)
{
}

KSecretServiceCollection::Entry::Entry(const KSecretServiceCollection::Entry& other)
    : d(other.d)
{
}

KSecretServiceCollection::Entry::Entry(const QString& path, const QString& session)
{
    d = new Data;
    OrgFreedesktopSecretItemInterface interface("org.freedesktop.Secret", path, QDBusConnection::sessionBus());

    d->attributes = interface.attributes();
    d->label = interface.label();
    d->created = QDateTime::fromMSecsSinceEpoch(interface.created());
    d->lastModified = QDateTime::fromMSecsSinceEpoch(interface.modified());

    QDBusPendingReply< Secret > reply = interface.GetSecret(QDBusObjectPath(session));
    reply.waitForFinished();

    d->secret = reply.value();
}

KSecretServiceCollection::Entry::~Entry()
{

}

QMap< QString, QString > KSecretServiceCollection::Entry::attributes() const
{
    return d->attributes;
}

QDateTime KSecretServiceCollection::Entry::created() const
{
    return d->created;
}

QString KSecretServiceCollection::Entry::label() const
{
    return d->label;
}

QDateTime KSecretServiceCollection::Entry::lastModified() const
{
    return d->lastModified;
}

QByteArray KSecretServiceCollection::Entry::secret() const
{
    return d->secret.value();
}

KSecretServiceCollection::KSecretServiceCollection(const QString& path, KSecretService* parent)
    : QObject(parent)
    , d(new Private)
{
    d->kss = parent;
    d->collectionInterface = new OrgFreedesktopSecretCollectionInterface("org.freedesktop.Secret",
                                                                         path,
                                                                         QDBusConnection::sessionBus(),
                                                                         this);
}

KSecretServiceCollection::~KSecretServiceCollection()
{
    delete d;
}

QDateTime KSecretServiceCollection::created() const
{
    return QDateTime::fromMSecsSinceEpoch(d->collectionInterface->created());
}

KJob* KSecretServiceCollection::deleteEntry(const QString& label)
{
    return new DeleteEntryJob(label, this);
}

QStringList KSecretServiceCollection::entries() const
{
    // Obtain the item labels
    QStringList itemLabels;
    foreach (const QDBusObjectPath &path, d->collectionInterface->items()) {
        OrgFreedesktopSecretItemInterface item("org.freedesktop.Secret",
                                               path.path(),
                                               QDBusConnection::sessionBus());
        if (item.isValid()) {
            itemLabels << item.label();
        }
    }
    return itemLabels;
}

QString KSecretServiceCollection::label() const
{
    return d->collectionInterface->label();
}

QDateTime KSecretServiceCollection::lastModified() const
{
    return QDateTime::fromMSecsSinceEpoch(d->collectionInterface->modified());
}

ReadEntryJob* KSecretServiceCollection::readEntry(const QString& label)
{
    return new ReadEntryJob(label, this);
}

KJob* KSecretServiceCollection::writeEntry(const QString& label, const QByteArray& secret, const QVariantMap &attributes)
{
    return new WriteEntryJob(label, secret, attributes, this);
}

#include "ksecretservicecollection.moc"
#include "ksecretservicecollection_p.moc"
