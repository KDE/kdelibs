/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>


    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <QMap>
#include <QStringList>

#include <solid/ifaces/bluetoothremotedevice.h>

#include "frontendobject_p.h"

#include "soliddefs_p.h"
#include "bluetoothmanager.h"
#include "bluetoothinterface.h"

namespace Solid
{
    class BluetoothRemoteDevicePrivate : public FrontendObjectPrivate
    {
    public:
        BluetoothRemoteDevicePrivate(QObject *parent)
            : FrontendObjectPrivate(parent) { }

        void setBackendObject(QObject *object);
    };
}

Solid::BluetoothRemoteDevice::BluetoothRemoteDevice(QObject *backendObject)
    : QObject(), d_ptr(new BluetoothRemoteDevicePrivate(this))
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(backendObject);
}

Solid::BluetoothRemoteDevice::BluetoothRemoteDevice(const BluetoothRemoteDevice &device)
    : QObject(), d_ptr(new BluetoothRemoteDevicePrivate(this))
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(device.d_ptr->backendObject());
}

Solid::BluetoothRemoteDevice::~BluetoothRemoteDevice()
{}

Solid::BluetoothRemoteDevice &Solid::BluetoothRemoteDevice::operator=(const Solid::BluetoothRemoteDevice & dev)
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(dev.d_ptr->backendObject());

    return *this;
}

QString Solid::BluetoothRemoteDevice::ubi() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), ubi());
}

QString Solid::BluetoothRemoteDevice::address() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), address());
}

bool Solid::BluetoothRemoteDevice::isConnected() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), false, isConnected());
}

QString Solid::BluetoothRemoteDevice::version() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), version());
}

QString Solid::BluetoothRemoteDevice::revision() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), revision());
}

QString Solid::BluetoothRemoteDevice::manufacturer() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), manufacturer());
}

QString Solid::BluetoothRemoteDevice::company() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), company());
}

QString Solid::BluetoothRemoteDevice::majorClass() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), majorClass());
}

QString Solid::BluetoothRemoteDevice::minorClass() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), minorClass());
}

QStringList Solid::BluetoothRemoteDevice::serviceClasses() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QStringList(), serviceClasses());
}

QString Solid::BluetoothRemoteDevice::name() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), name());
}

QString Solid::BluetoothRemoteDevice::alias() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), alias());
}

QString Solid::BluetoothRemoteDevice::lastSeen() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), lastSeen());
}

QString Solid::BluetoothRemoteDevice::lastUsed() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), lastUsed());
}

bool Solid::BluetoothRemoteDevice::hasBonding() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), false, hasBonding());
}

int Solid::BluetoothRemoteDevice::pinCodeLength() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), 0, pinCodeLength());
}

int Solid::BluetoothRemoteDevice::encryptionKeySize() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), 0, encryptionKeySize());
}

KJob *Solid::BluetoothRemoteDevice::createBonding()
{
    Q_D(BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), 0, createBonding());
}

void Solid::BluetoothRemoteDevice::setAlias(const QString &alias)
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), setAlias(alias));
}

void Solid::BluetoothRemoteDevice::clearAlias()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), clearAlias());
}

void Solid::BluetoothRemoteDevice::disconnect()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), disconnect());
}

void Solid::BluetoothRemoteDevice::cancelBondingProcess()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), cancelBondingProcess());
}

void Solid::BluetoothRemoteDevice::removeBonding()
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), removeBonding());
}

void Solid::BluetoothRemoteDevicePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(classChanged(uint)),
                         parent(), SIGNAL(classChanged(uint)));
        QObject::connect(object, SIGNAL(nameChanged(const QString&)),
                         parent(), SIGNAL(nameChanged(const QString&)));
        QObject::connect(object, SIGNAL(nameResolvingFailed()),
                         parent(), SIGNAL(nameResolvingFailed()));
        QObject::connect(object, SIGNAL(aliasChanged(const QString &)),
                         parent(), SIGNAL(aliasChanged(const QString &)));
        QObject::connect(object, SIGNAL(aliasCleared()),
                         parent(), SIGNAL(aliasCleared()));
        QObject::connect(object, SIGNAL(connected()),
                         parent(), SIGNAL(connected()));
        QObject::connect(object, SIGNAL(requestDisconnection()),
                         parent(), SIGNAL(requestDisconnection()));
        QObject::connect(object, SIGNAL(disconnected()),
                         parent(), SIGNAL(disconnected()));
        QObject::connect(object, SIGNAL(bondingCreated()),
                         parent(), SIGNAL(bondingCreated()));
        QObject::connect(object, SIGNAL(bondingRemoved()),
                         parent(), SIGNAL(bondingRemoved()));

    }
}

#include "bluetoothremotedevice.moc"
