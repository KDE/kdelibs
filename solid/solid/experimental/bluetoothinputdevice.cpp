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

#include <solid/experimental/ifaces/bluetoothinputdevice.h>

#include "frontendobject_p.h"

#include "../soliddefs_p.h"

#include "bluetoothinputdevice.h"

namespace SolidExperimental
{
    class BluetoothInputDevicePrivate : public FrontendObjectPrivate
    {
    public:
        BluetoothInputDevicePrivate(QObject *parent)
            : FrontendObjectPrivate(parent) { }

        void setBackendObject(QObject *object);
    };
}

SolidExperimental::BluetoothInputDevice::BluetoothInputDevice(QObject *backendObject)
    : QObject(), d(new BluetoothInputDevicePrivate(this))
{
    d->setBackendObject(backendObject);
}

SolidExperimental::BluetoothInputDevice::BluetoothInputDevice(const BluetoothInputDevice &device)
    : QObject(), d(new BluetoothInputDevicePrivate(this))
{
    d->setBackendObject(device.d->backendObject());
}

SolidExperimental::BluetoothInputDevice::~BluetoothInputDevice()
{}

SolidExperimental::BluetoothInputDevice &SolidExperimental::BluetoothInputDevice::operator=(const SolidExperimental::BluetoothInputDevice & dev)
{
    d->setBackendObject(dev.d->backendObject());

    return *this;
}

QString SolidExperimental::BluetoothInputDevice::ubi() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), QString(), ubi());
}

bool SolidExperimental::BluetoothInputDevice::isConnected() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), false, isConnected());
}

QString SolidExperimental::BluetoothInputDevice::name() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), QString(), name());
}

QString SolidExperimental::BluetoothInputDevice::address() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), QString(), address());
}

QString SolidExperimental::BluetoothInputDevice::productID() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), QString(), productID());
}

QString SolidExperimental::BluetoothInputDevice::vendorID() const
{
    return_SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), QString(), vendorID());
}

void SolidExperimental::BluetoothInputDevice::slotConnect()
{
    SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), slotConnect());
}

void SolidExperimental::BluetoothInputDevice::slotDisconnect()
{
    SOLID_CALL(Ifaces::BluetoothInputDevice*, d->backendObject(), slotDisconnect());
}

void SolidExperimental::BluetoothInputDevicePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(connected()),
                         parent(), SIGNAL(connected()));
        QObject::connect(object, SIGNAL(disconnected()),
                         parent(), SIGNAL(disconnected()));
    }
}

#include "bluetoothinputdevice.moc"
