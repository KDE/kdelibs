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

#ifndef KSECRETSERVICECOLLECTION_P_H
#define KSECRETSERVICECOLLECTION_P_H

#include "ksecretservicecollection.h"

#include <QtCore/QStringList>

#include "../daemon/frontend/secret/adaptors/dbustypes.h"

// Jobs

class QDBusPendingCallWatcher;
class SearchEntriesJob : public KJob
{
    Q_OBJECT

public:
    explicit SearchEntriesJob(const QString &label, const QMap< QString, QString > &properties,
                              KSecretServiceCollection* parent);
    virtual ~SearchEntriesJob();

    virtual void start();

    QStringList result() const;

private Q_SLOTS:
    void run();
    void onReplyFinished(QDBusPendingCallWatcher *reply);

private:
    KSecretServiceCollection *m_kssc;
    QString m_label;
    QMap< QString, QString > m_properties;

    QStringList m_result;
};

class WriteEntryJob : public KJob
{
    Q_OBJECT

public:
    explicit WriteEntryJob(const QByteArray &secret,
                           const QVariantMap &properties, KSecretServiceCollection *parent);
    virtual ~WriteEntryJob();

    virtual void start();

private Q_SLOTS:
    void run();
    void onReplyFinished(QDBusPendingCallWatcher *reply);

private:
    KSecretServiceCollection *m_kssc;

    QByteArray m_secret;
    QVariantMap m_properties;
};

class DeleteEntryJob : public KJob
{
    Q_OBJECT

public:
    explicit DeleteEntryJob(const QString &label, KSecretServiceCollection *parent);
    virtual ~DeleteEntryJob();

    virtual void start();

private Q_SLOTS:
    void run();
    void onSearchFinished(KJob *job);
    void onReplyFinished(QDBusPendingCallWatcher *reply);

private:
    KSecretServiceCollection *m_kssc;

    QString m_label;
};

#endif
