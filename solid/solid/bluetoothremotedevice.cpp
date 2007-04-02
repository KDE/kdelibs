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

Solid::BluetoothRemoteDevice::BluetoothRemoteDevice(QObject *backendObject)
        : FrontendObject()
{
    registerBackendObject(backendObject);
}

Solid::BluetoothRemoteDevice::BluetoothRemoteDevice(const BluetoothRemoteDevice &device)
        : FrontendObject()
{
    registerBackendObject(device.backendObject());
}

Solid::BluetoothRemoteDevice::~BluetoothRemoteDevice()
{}

Solid::BluetoothRemoteDevice &Solid::BluetoothRemoteDevice::operator=(const Solid::BluetoothRemoteDevice & dev)
{
    unregisterBackendObject();
    registerBackendObject(dev.backendObject());

    return *this;
}

QString Solid::BluetoothRemoteDevice::ubi() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), ubi());
}

QString Solid::BluetoothRemoteDevice::address() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), address());
}

bool Solid::BluetoothRemoteDevice::isConnected() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), false, isConnected());
}

QString Solid::BluetoothRemoteDevice::version() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), version());
}

QString Solid::BluetoothRemoteDevice::revision() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), revision());
}

QString Solid::BluetoothRemoteDevice::manufacturer() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), manufacturer());
}

QString Solid::BluetoothRemoteDevice::company() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), company());
}

QString Solid::BluetoothRemoteDevice::majorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), majorClass());
}

QString Solid::BluetoothRemoteDevice::minorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), minorClass());
}

QStringList Solid::BluetoothRemoteDevice::serviceClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QStringList(), serviceClasses());
}

QString Solid::BluetoothRemoteDevice::name() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), name());
}

QString Solid::BluetoothRemoteDevice::alias() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), alias());
}

QString Solid::BluetoothRemoteDevice::lastSeen() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), lastSeen());
}

QString Solid::BluetoothRemoteDevice::lastUsed() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), QString(), lastUsed());
}

bool Solid::BluetoothRemoteDevice::hasBonding() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), false, hasBonding());
}

int Solid::BluetoothRemoteDevice::pinCodeLength() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), 0, pinCodeLength());
}

int Solid::BluetoothRemoteDevice::encryptionKeySize() const
{
    return_SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), 0, encryptionKeySize());
}

void Solid::BluetoothRemoteDevice::setAlias(const QString &alias)
{
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), setAlias(alias));
}

void Solid::BluetoothRemoteDevice::clearAlias()
{
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), clearAlias());
}

void Solid::BluetoothRemoteDevice::disconnect()
{
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), disconnect());
}

void Solid::BluetoothRemoteDevice::createBonding()
{
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), createBonding());
}

void Solid::BluetoothRemoteDevice::cancelBondingProcess()
{
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), cancelBondingProcess());
}

void Solid::BluetoothRemoteDevice::removeBonding()
{
    SOLID_CALL(Ifaces::BluetoothRemoteDevice*, backendObject(), removeBonding());
}

void Solid::BluetoothRemoteDevice::slotDestroyed(QObject *object)
{
    if (object == backendObject()) {
        FrontendObject::slotDestroyed(object);
    }
}

void Solid::BluetoothRemoteDevice::registerBackendObject(QObject *backendObject)
{
    setBackendObject(backendObject);

    if (backendObject) {
        connect(backendObject, SIGNAL(classChanged(uint)),
                this, SIGNAL(classChanged(uint)));
        connect(backendObject, SIGNAL(nameChanged(const QString&)),
                this, SIGNAL(nameChanged(const QString&)));
        connect(backendObject, SIGNAL(nameResolvingFailed()),
                this, SIGNAL(nameResolvingFailed()));
        connect(backendObject, SIGNAL(aliasChanged(const QString &)),
                this, SIGNAL(aliasChanged(const QString &)));
        connect(backendObject, SIGNAL(aliasCleared()),
                this, SIGNAL(aliasCleared()));
        connect(backendObject, SIGNAL(connected()),
                this, SIGNAL(connected()));
        connect(backendObject, SIGNAL(requestDisconnection()),
                this, SIGNAL(requestDisconnection()));
        connect(backendObject, SIGNAL(disconnected()),
                this, SIGNAL(disconnected()));
        connect(backendObject, SIGNAL(bondingCreated()),
                this, SIGNAL(bondingCreated()));
        connect(backendObject, SIGNAL(bondingRemoved()),
                this, SIGNAL(bondingRemoved()));

    }
}

void Solid::BluetoothRemoteDevice::unregisterBackendObject()
{
    setBackendObject(0);
}

#include "bluetoothremotedevice.moc"
