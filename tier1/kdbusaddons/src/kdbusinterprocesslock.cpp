/* This file is part of the KDE

   Copyright (C) 2009 Tobias Koenig (tokoe@kde.org)
   Copyright (C) 2011 Kevin Ottens (ervin@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) version 3, or any
   later version accepted by the membership of KDE e.V. (or its
   successor approved by the membership of KDE e.V.), which shall
   act as a proxy defined in Section 6 of version 3 of the license.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kdbusinterprocesslock.h"

#include <QtCore/QEventLoop>
#include <QtDBus/QDBusConnectionInterface>

#include <QtCore/QDebug>

class KDBusInterProcessLockPrivate
{
public:
    KDBusInterProcessLockPrivate(const QString &resource, KDBusInterProcessLock *parent)
        : m_resource(resource), m_parent(parent)
    {
        m_serviceName = QString::fromLatin1("org.kde.private.lock-%1").arg(m_resource);

        m_parent->connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceRegistered(const QString&)),
                          m_parent, SLOT(_k_serviceRegistered(const QString&)));
    }

    ~KDBusInterProcessLockPrivate()
    {
    }

    void _k_serviceRegistered(const QString &service)
    {
        if (service == m_serviceName)
            emit m_parent->lockGranted(m_parent);
    }

    QString m_resource;
    QString m_serviceName;
    KDBusInterProcessLock *m_parent;
};

KDBusInterProcessLock::KDBusInterProcessLock(const QString &resource)
    : d(new KDBusInterProcessLockPrivate(resource, this))
{
}

KDBusInterProcessLock::~KDBusInterProcessLock()
{
    delete d;
}

QString KDBusInterProcessLock::resource() const
{
    return d->m_resource;
}

void KDBusInterProcessLock::lock()
{
    QDBusConnection::sessionBus().interface()->registerService(d->m_serviceName,
                                                               QDBusConnectionInterface::QueueService,
                                                               QDBusConnectionInterface::DontAllowReplacement);
}

void KDBusInterProcessLock::unlock()
{
    QDBusConnection::sessionBus().interface()->unregisterService(d->m_serviceName);
}

void KDBusInterProcessLock::waitForLockGranted()
{
    QEventLoop loop;
    connect(this, SIGNAL(lockGranted(KDBusInterProcessLock*)), &loop, SLOT(quit()));
    loop.exec();
}

#include "moc_kdbusinterprocesslock.cpp"
