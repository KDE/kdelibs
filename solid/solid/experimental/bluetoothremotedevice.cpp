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

#include <solid/experimental/ifaces/bluetoothremotedevice.h>

#include "frontendobject_p.h"

#include "../soliddefs_p.h"
#include "bluetoothmanager.h"
#include "bluetoothinterface.h"

namespace SolidExperimental
{
    class BluetoothRemoteDevicePrivate : public FrontendObjectPrivate
    {
    public:
        BluetoothRemoteDevicePrivate(QObject *parent)
            : FrontendObjectPrivate(parent) { }

        void setBackendObject(QObject *object);
    };
}

SolidExperimental::BluetoothRemoteDevice::BluetoothRemoteDevice(QObject *backendObject)
    : QObject(), d_ptr(new BluetoothRemoteDevicePrivate(this))
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(backendObject);
}

SolidExperimental::BluetoothRemoteDevice::BluetoothRemoteDevice(const BluetoothRemoteDevice &device)
    : QObject(), d_ptr(new BluetoothRemoteDevicePrivate(this))
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(device.d_ptr->backendObject());
}

SolidExperimental::BluetoothRemoteDevice::~BluetoothRemoteDevice()
{}

SolidExperimental::BluetoothRemoteDevice &SolidExperimental::BluetoothRemoteDevice::operator=(const SolidExperimental::BluetoothRemoteDevice & dev)
{
    Q_D(BluetoothRemoteDevice);
    d->setBackendObject(dev.d_ptr->backendObject());

    return *this;
}

QString SolidExperimental::BluetoothRemoteDevice::ubi() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), ubi());
}

QString SolidExperimental::BluetoothRemoteDevice::address() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), address());
}

bool SolidExperimental::BluetoothRemoteDevice::isConnected() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), false, isConnected());
}

QString SolidExperimental::BluetoothRemoteDevice::version() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), version());
}

QString SolidExperimental::BluetoothRemoteDevice::revision() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), revision());
}

QString SolidExperimental::BluetoothRemoteDevice::manufacturer() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), manufacturer());
}

QString SolidExperimental::BluetoothRemoteDevice::company() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), company());
}

QString SolidExperimental::BluetoothRemoteDevice::majorClass() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), majorClass());
}

QString SolidExperimental::BluetoothRemoteDevice::minorClass() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), minorClass());
}

QStringList SolidExperimental::BluetoothRemoteDevice::serviceClasses() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QStringList(), serviceClasses());
}

QString SolidExperimental::BluetoothRemoteDevice::name() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), name());
}

QString SolidExperimental::BluetoothRemoteDevice::alias() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), alias());
}

QString SolidExperimental::BluetoothRemoteDevice::lastSeen() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), lastSeen());
}

QString SolidExperimental::BluetoothRemoteDevice::lastUsed() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), QString(), lastUsed());
}

bool SolidExperimental::BluetoothRemoteDevice::hasBonding() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), false, hasBonding());
}

int SolidExperimental::BluetoothRemoteDevice::pinCodeLength() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), 0, pinCodeLength());
}

int SolidExperimental::BluetoothRemoteDevice::encryptionKeySize() const
{
    Q_D(const BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), 0, encryptionKeySize());
}

KJob *SolidExperimental::BluetoothRemoteDevice::createBonding()
{
    Q_D(BluetoothRemoteDevice);
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), 0, createBonding());
}

void SolidExperimental::BluetoothRemoteDevice::setAlias(const QString &alias)
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), setAlias(alias));
}

void SolidExperimental::BluetoothRemoteDevice::clearAlias()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), clearAlias());
}

void SolidExperimental::BluetoothRemoteDevice::disconnect()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), disconnect());
}

void SolidExperimental::BluetoothRemoteDevice::cancelBondingProcess()
{
    Q_D(BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), cancelBondingProcess());
}

void SolidExperimental::BluetoothRemoteDevice::removeBonding()
{
    Q_D(const BluetoothRemoteDevice);
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, d->backendObject(), removeBonding());
}

void SolidExperimental::BluetoothRemoteDevicePrivate::setBackendObject(QObject *object)
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
