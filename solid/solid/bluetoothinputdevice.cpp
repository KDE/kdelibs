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

#include <solid/ifaces/bluetoothinputdevice.h>

#include "frontendobject_p.h"

#include "soliddefs_p.h"

#include "bluetoothinputdevice.h"

Solid::BluetoothInputDevice::BluetoothInputDevice(QObject *backendObject)
        : FrontendObject()
{
    registerBackendObject(backendObject);
}

Solid::BluetoothInputDevice::BluetoothInputDevice(const BluetoothInputDevice &device)
        : FrontendObject()
{
    registerBackendObject(device.backendObject());
}

Solid::BluetoothInputDevice::~BluetoothInputDevice()
{}

Solid::BluetoothInputDevice &Solid::BluetoothInputDevice::operator=(const Solid::BluetoothInputDevice & dev)
{
    unregisterBackendObject();
    registerBackendObject(dev.backendObject());

    return *this;
}

QString Solid::BluetoothInputDevice::ubi() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), QString(), ubi());
}

bool Solid::BluetoothInputDevice::isConnected() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), false, isConnected());
}

QString Solid::BluetoothInputDevice::name() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), QString(), name());
}

QString Solid::BluetoothInputDevice::address() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), QString(), address());
}

QString Solid::BluetoothInputDevice::productID() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), QString(), productID());
}

QString Solid::BluetoothInputDevice::vendorID() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), QString(), vendorID());
}

void Solid::BluetoothInputDevice::slotConnect()
{
    SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), slotConnect());
}

void Solid::BluetoothInputDevice::slotDisconnect()
{
    SOLID_CALL(Ifaces::BluetoothInputDevice*, backendObject(), slotDisconnect());
}

void Solid::BluetoothInputDevice::slotDestroyed(QObject *object)
{
    if (object == backendObject()) {
        FrontendObject::slotDestroyed(object);
    }
}

void Solid::BluetoothInputDevice::registerBackendObject(QObject *backendObject)
{
    setBackendObject(backendObject);

    if (backendObject) {
        connect(backendObject, SIGNAL(connected()),
                this, SIGNAL(connected()));
        connect(backendObject, SIGNAL(disconnected()),
                this, SIGNAL(disconnected()));
    }
}

void Solid::BluetoothInputDevice::unregisterBackendObject()
{
    setBackendObject(0);
}

#include "bluetoothinputdevice.moc"
