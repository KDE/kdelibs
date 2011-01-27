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

#ifndef KSECRETSERVICE_P_H
#define KSECRETSERVICE_P_H

#include "ksecretservice.h"

#include <kjob.h>

class OrgFreedesktopSecretSessionInterface;
class OrgFreedesktopSecretServiceInterface;
class KSecretService::Private {
public:
    Private() : serviceInterface(0), sessionInterface(0) {}
    OrgFreedesktopSecretServiceInterface *serviceInterface;
    OrgFreedesktopSecretSessionInterface *sessionInterface;
};

// Jobs

class QDBusPendingCallWatcher;
class CreateCollectionJob : public KJob
{
    Q_OBJECT
public:
    explicit CreateCollectionJob(const QString& label, const QVariantMap& properties, KSecretService* parent);
    virtual ~CreateCollectionJob();

    virtual void start();

private Q_SLOTS:
    void run();
    void onReplyFinished(QDBusPendingCallWatcher *reply);

private:
    KSecretService *m_kss;
    QString m_label;
    QVariantMap m_properties;
};

class DeleteCollectionJob : public KJob
{
    Q_OBJECT
public:
    explicit DeleteCollectionJob(const QString& label, KSecretService* parent);
    virtual ~DeleteCollectionJob();

    virtual void start();

private Q_SLOTS:
    void run();
    void onReplyFinished(QDBusPendingCallWatcher *reply);

private:
    KSecretService *m_kss;
    QString m_label;
};

#endif
