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
#include <QPair>
#include <QStringList>

#include <kdebug.h>

#include <solid/ifaces/bluetoothinterface.h>

#include "frontendobject_p.h"

#include "soliddefs_p.h"
#include "bluetoothmanager.h"
#include "bluetoothinterface.h"
#include "bluetoothremotedevice.h"

namespace Solid
{
class BluetoothInterfacePrivate : public FrontendObjectPrivate
{
public:
    mutable QMap<QString, BluetoothRemoteDevice*> remoteDeviceMap;
    mutable BluetoothRemoteDevice invalidDevice;
};
}

Solid::BluetoothInterface::BluetoothInterface()
        : FrontendObject(*new BluetoothInterfacePrivate)
{}

Solid::BluetoothInterface::BluetoothInterface(const QString &ubi)
        : FrontendObject(*new BluetoothInterfacePrivate)
{
    const BluetoothInterface &device = BluetoothManager::self().findBluetoothInterface(ubi);
    registerBackendObject(device.backendObject());
}

Solid::BluetoothInterface::BluetoothInterface(QObject *backendObject)
        : FrontendObject(*new BluetoothInterfacePrivate)
{
    registerBackendObject(backendObject);
}

Solid::BluetoothInterface::BluetoothInterface(const BluetoothInterface &device)
        : FrontendObject(*new BluetoothInterfacePrivate)
{
    registerBackendObject(device.backendObject());
}

Solid::BluetoothInterface::~BluetoothInterface()
{
    Q_D(BluetoothInterface);

    typedef QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> DeviceIfacePair;

    foreach(QObject *remoteDevice, d->remoteDeviceMap) {
        delete remoteDevice;
    }
}

Solid::BluetoothInterface &Solid::BluetoothInterface::operator=(const Solid::BluetoothInterface & dev)
{
    unregisterBackendObject();
    registerBackendObject(dev.backendObject());

    return *this;
}

QString Solid::BluetoothInterface::ubi() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), ubi());
}

Solid::BluetoothRemoteDevice *Solid::BluetoothInterface::findBluetoothRemoteDevice(const QString &ubi) const
{
    Q_D(const BluetoothInterface);

    BluetoothRemoteDevice *remoteDevice = findRegisteredBluetoothRemoteDevice(ubi);

    if (remoteDevice != 0) {
        return remoteDevice;
    } else {
        return &(d->invalidDevice);
    }
}

QString Solid::BluetoothInterface::address() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), address());
}

QString Solid::BluetoothInterface::version() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), version());
}

QString Solid::BluetoothInterface::revision() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), revision());
}

QString Solid::BluetoothInterface::manufacturer() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), manufacturer());
}

QString Solid::BluetoothInterface::company() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), company());
}

QString Solid::BluetoothInterface::mode() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), mode());
}

int Solid::BluetoothInterface::discoverableTimeout() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), 0, discoverableTimeout());
}

bool Solid::BluetoothInterface::isDiscoverable() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), false, isDiscoverable());
}

Solid::BluetoothRemoteDeviceList Solid::BluetoothInterface::listConnections() const
{
    BluetoothRemoteDeviceList list;
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface*>(backendObject());

    if (backend == 0) return list;

    QStringList ubis = backend->listConnections();

    foreach(const QString &ubi, ubis) {
        BluetoothRemoteDevice *remoteDevice = findRegisteredBluetoothRemoteDevice(ubi);

        if (remoteDevice != 0) {
            list.append(remoteDevice);
        }
    }

    return list;
}

QString Solid::BluetoothInterface::majorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), majorClass());
}

QStringList Solid::BluetoothInterface::listAvailableMinorClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QStringList(), listAvailableMinorClasses());
}

QString Solid::BluetoothInterface::minorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), minorClass());
}

QStringList Solid::BluetoothInterface::serviceClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QStringList(), serviceClasses());
}

QString Solid::BluetoothInterface::name() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QString(), name());
}

QStringList Solid::BluetoothInterface::listBondings() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QStringList(), listBondings());
}

bool Solid::BluetoothInterface::isPeriodicDiscovery() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), false, isPeriodicDiscovery());
}

bool Solid::BluetoothInterface::isPeriodicDiscoveryNameResolving() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), false,
                      isPeriodicDiscoveryNameResolving());
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList Solid::BluetoothInterface::listRemoteDevices() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QStringList(),
                      listRemoteDevices());
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList Solid::BluetoothInterface::listRecentRemoteDevices(const QString &date) const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), QStringList(),
                      listRecentRemoteDevices(date));
}

/***************************************************************/

void Solid::BluetoothInterface::setMode(const QString &mode)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), setMode(mode));
}

void Solid::BluetoothInterface::setDiscoverableTimeout(int timeout)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), setDiscoverableTimeout(timeout));
}

void Solid::BluetoothInterface::setMinorClass(const QString &minor)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), setMinorClass(minor));
}

void Solid::BluetoothInterface::setName(const QString &name)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), setName(name));
}

void Solid::BluetoothInterface::discoverDevices()
{
    kDebug() << k_funcinfo << endl;
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), discoverDevices());
}

void Solid::BluetoothInterface::discoverDevicesWithoutNameResolving()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), discoverDevicesWithoutNameResolving());
}

void Solid::BluetoothInterface::cancelDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), cancelDiscovery());
}

void Solid::BluetoothInterface::startPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), startPeriodicDiscovery());
}

void Solid::BluetoothInterface::stopPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), stopPeriodicDiscovery());
}

void Solid::BluetoothInterface::setPeriodicDiscoveryNameResolving(bool resolveNames)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, backendObject(), setPeriodicDiscoveryNameResolving(resolveNames));
}

void Solid::BluetoothInterface::slotDestroyed(QObject *object)
{
    if (object == backendObject()) {
        FrontendObject::slotDestroyed(object);
    }
}

void Solid::BluetoothInterface::registerBackendObject(QObject *backendObject)
{
    setBackendObject(backendObject);

    if (backendObject) {
        connect(backendObject, SIGNAL(modeChanged(const QString&)),
                this, SIGNAL(modeChanged(const QString&)));
        connect(backendObject, SIGNAL(discoverableTimeoutChanged(int)),
                this, SIGNAL(discoverableTimeoutChanged(int)));
        connect(backendObject, SIGNAL(minorClassChanged(const QString&)),
                this, SIGNAL(minorClassChanged(const QString&)));
        connect(backendObject, SIGNAL(nameChanged(const QString&)),
                this, SIGNAL(nameChanged(const QString&)));
        connect(backendObject, SIGNAL(discoveryStarted()),
                this, SIGNAL(discoveryStarted()));
        connect(backendObject, SIGNAL(discoveryCompleted()),
                this, SIGNAL(discoveryCompleted()));
        connect(backendObject, SIGNAL(remoteDeviceFound(const QString&, int, int)),
                this, SIGNAL(remoteDeviceFound(const QString&, int, int)));
        connect(backendObject, SIGNAL(remoteDeviceDisappeared(const QString&)),
                this, SIGNAL(remoteDeviceDisappeared(const QString&)));
    }
}

void Solid::BluetoothInterface::unregisterBackendObject()
{
    setBackendObject(0);
}

Solid::BluetoothRemoteDevice *Solid::BluetoothInterface::findRegisteredBluetoothRemoteDevice(const QString &ubi) const
{
    Q_D(const BluetoothInterface);

    BluetoothRemoteDevice *remoteDevice = 0;

    if (d->remoteDeviceMap.contains(ubi)) {
        remoteDevice = d->remoteDeviceMap[ubi];
    } else {
        Ifaces::BluetoothInterface *device = qobject_cast<Ifaces::BluetoothInterface*>(backendObject());

        if (device != 0) {
            QObject *iface = device->createBluetoothRemoteDevice(ubi);
            remoteDevice = new BluetoothRemoteDevice(iface);

            if (remoteDevice != 0) {
                d->remoteDeviceMap[ubi] = remoteDevice;
            }
        }
    }

    return remoteDevice;
}

#include "bluetoothinterface.moc"
