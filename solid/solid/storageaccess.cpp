/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "storageaccess.h"
#include "storageaccess_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/storageaccess.h>

Solid::StorageAccess::StorageAccess(QObject *backendObject)
    : DeviceInterface(*new StorageAccessPrivate(), backendObject)
{
    connect(backendObject, SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)),
            this, SIGNAL(setupDone(Solid::ErrorType, QVariant, const QString &)));
    connect(backendObject, SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)),
            this, SIGNAL(teardownDone(Solid::ErrorType, QVariant, const QString &)));
    connect(backendObject, SIGNAL(setupRequested(const QString &)),
            this, SIGNAL(setupRequested(const QString &)));
    connect(backendObject, SIGNAL(teardownRequested(const QString &)),
            this, SIGNAL(teardownRequested(const QString &)));

    connect(backendObject, SIGNAL(accessibilityChanged(bool, const QString &)),
            this, SIGNAL(accessibilityChanged(bool, const QString &)));
}

Solid::StorageAccess::StorageAccess(StorageAccessPrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{
    connect(backendObject, SIGNAL(setupDone(Solid::StorageAccess::SetupResult, QVariant, const QString &)),
            this, SIGNAL(setupDone(Solid::StorageAccess::SetupResult, QVariant, const QString &)));
    connect(backendObject, SIGNAL(teardownDone(Solid::StorageAccess::TeardownResult, QVariant, const QString &)),
            this, SIGNAL(teardownDone(Solid::StorageAccess::TeardownResult, QVariant, const QString &)));
    connect(backendObject, SIGNAL(setupRequested(const QString &)),
            this, SIGNAL(setupRequested(const QString &)));
    connect(backendObject, SIGNAL(teardownRequested(const QString &)),
            this, SIGNAL(teardownRequested(const QString &)));


    connect(backendObject, SIGNAL(accessibilityChanged(bool, const QString &)),
            this, SIGNAL(accessibilityChanged(bool, const QString &)));
}

Solid::StorageAccess::~StorageAccess()
{

}

bool Solid::StorageAccess::isAccessible() const
{
    Q_D(const StorageAccess);
    return_SOLID_CALL(Ifaces::StorageAccess *, d->backendObject(), false, isAccessible());
}

QString Solid::StorageAccess::filePath() const
{
    Q_D(const StorageAccess);
    return_SOLID_CALL(Ifaces::StorageAccess *, d->backendObject(), QString(), filePath());
}

bool Solid::StorageAccess::setup()
{
    Q_D(StorageAccess);
    return_SOLID_CALL(Ifaces::StorageAccess *, d->backendObject(), false, setup());
}

bool Solid::StorageAccess::teardown()
{
    Q_D(StorageAccess);
    return_SOLID_CALL(Ifaces::StorageAccess *, d->backendObject(), false, teardown());
}

bool Solid::StorageAccess::isIgnored() const
{
    Q_D(const StorageAccess);
    return_SOLID_CALL(Ifaces::StorageAccess *, d->backendObject(), true, isIgnored());
}

#include "storageaccess.moc"
