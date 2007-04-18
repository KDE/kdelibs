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
    BluetoothInterfacePrivate(QObject *parent)
        : FrontendObjectPrivate(parent) { }

    void setBackendObject(QObject *object);

    QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> findRegisteredBluetoothRemoteDevice(const QString &ubi) const;

    mutable QMap<QString, QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> > remoteDeviceMap;
    mutable BluetoothRemoteDevice invalidDevice;
};
}

Solid::BluetoothInterface::BluetoothInterface()
        : QObject(), d(new BluetoothInterfacePrivate(this))
{}

Solid::BluetoothInterface::BluetoothInterface(const QString &ubi)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    const BluetoothInterface &device = BluetoothManager::self().findBluetoothInterface(ubi);
    d->setBackendObject(device.d->backendObject());
}

Solid::BluetoothInterface::BluetoothInterface(QObject *backendObject)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    d->setBackendObject(backendObject);
}

Solid::BluetoothInterface::BluetoothInterface(const BluetoothInterface &device)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    d->setBackendObject(device.d->backendObject());
}

Solid::BluetoothInterface::~BluetoothInterface()
{
    // Delete all the interfaces, they are now outdated
    typedef QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> BluetoothRemoteDeviceIfacePair;

    // Delete all the devices, they are now outdated
    foreach(const BluetoothRemoteDeviceIfacePair &pair, d->remoteDeviceMap.values()) {
        delete pair.first;
        delete pair.second;
    }
}

Solid::BluetoothInterface &Solid::BluetoothInterface::operator=(const Solid::BluetoothInterface & dev)
{
    d->setBackendObject(dev.d->backendObject());

    return *this;
}

QString Solid::BluetoothInterface::ubi() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), ubi());
}

const Solid::BluetoothRemoteDevice &Solid::BluetoothInterface::findBluetoothRemoteDevice(const QString &ubi) const
{
    QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> pair = d->findRegisteredBluetoothRemoteDevice(ubi);

    if (pair.first != 0) {
        return *pair.first;
    } else {
        return d->invalidDevice;
    }
}

QString Solid::BluetoothInterface::address() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), address());
}

QString Solid::BluetoothInterface::version() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), version());
}

QString Solid::BluetoothInterface::revision() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), revision());
}

QString Solid::BluetoothInterface::manufacturer() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), manufacturer());
}

QString Solid::BluetoothInterface::company() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), company());
}

Solid::BluetoothInterface::Mode Solid::BluetoothInterface::mode() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), Solid::BluetoothInterface::Off, mode());
}

int Solid::BluetoothInterface::discoverableTimeout() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), 0, discoverableTimeout());
}

bool Solid::BluetoothInterface::isDiscoverable() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), false, isDiscoverable());
}

Solid::BluetoothRemoteDeviceList Solid::BluetoothInterface::listConnections() const
{
    BluetoothRemoteDeviceList list;
    Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface*>(d->backendObject());

    if (backend == 0) return list;

    QStringList ubis = backend->listConnections();

    foreach(const QString &ubi, ubis) {
        BluetoothRemoteDevice remoteDevice = findBluetoothRemoteDevice(ubi);
        list.append(remoteDevice);
    }

    return list;
}

QString Solid::BluetoothInterface::majorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), majorClass());
}

QStringList Solid::BluetoothInterface::listAvailableMinorClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(), listAvailableMinorClasses());
}

QString Solid::BluetoothInterface::minorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), minorClass());
}

QStringList Solid::BluetoothInterface::serviceClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(), serviceClasses());
}

QString Solid::BluetoothInterface::name() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), name());
}

QStringList Solid::BluetoothInterface::listBondings() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(), listBondings());
}

bool Solid::BluetoothInterface::isPeriodicDiscoveryActive() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), false, isPeriodicDiscoveryActive());
}

bool Solid::BluetoothInterface::isPeriodicDiscoveryNameResolvingActive() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), false,
                      isPeriodicDiscoveryNameResolvingActive());
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList Solid::BluetoothInterface::listRemoteDevices() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(),
                      listRemoteDevices());
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList Solid::BluetoothInterface::listRecentRemoteDevices(const QDateTime &date) const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(),
                      listRecentRemoteDevices(date));
}

/***************************************************************/

void Solid::BluetoothInterface::setMode(const Solid::BluetoothInterface::Mode mode)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setMode(mode));
}

void Solid::BluetoothInterface::setDiscoverableTimeout(int timeout)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setDiscoverableTimeout(timeout));
}

void Solid::BluetoothInterface::setMinorClass(const QString &minor)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setMinorClass(minor));
}

void Solid::BluetoothInterface::setName(const QString &name)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setName(name));
}

void Solid::BluetoothInterface::discoverDevices()
{
    kDebug() << k_funcinfo << endl;
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), discoverDevices());
}

void Solid::BluetoothInterface::discoverDevicesWithoutNameResolving()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), discoverDevicesWithoutNameResolving());
}

void Solid::BluetoothInterface::cancelDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), cancelDiscovery());
}

void Solid::BluetoothInterface::startPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), startPeriodicDiscovery());
}

void Solid::BluetoothInterface::stopPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), stopPeriodicDiscovery());
}

void Solid::BluetoothInterface::setPeriodicDiscoveryNameResolving(bool resolveNames)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setPeriodicDiscoveryNameResolving(resolveNames));
}

void Solid::BluetoothInterfacePrivate::setBackendObject(QObject *object)
{
    setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(modeChanged(const QString&)),
                         parent(), SIGNAL(modeChanged(const QString&)));
        QObject::connect(object, SIGNAL(discoverableTimeoutChanged(int)),
                         parent(), SIGNAL(discoverableTimeoutChanged(int)));
        QObject::connect(object, SIGNAL(minorClassChanged(const QString&)),
                         parent(), SIGNAL(minorClassChanged(const QString&)));
        QObject::connect(object, SIGNAL(nameChanged(const QString&)),
                         parent(), SIGNAL(nameChanged(const QString&)));
        QObject::connect(object, SIGNAL(discoveryStarted()),
                         parent(), SIGNAL(discoveryStarted()));
        QObject::connect(object, SIGNAL(discoveryCompleted()),
                         parent(), SIGNAL(discoveryCompleted()));
        QObject::connect(object, SIGNAL(remoteDeviceFound(const QString&, int, int)),
                         parent(), SIGNAL(remoteDeviceFound(const QString&, int, int)));
        QObject::connect(object, SIGNAL(remoteDeviceDisappeared(const QString&)),
                         parent(), SIGNAL(remoteDeviceDisappeared(const QString&)));
    }
}

QPair<Solid::BluetoothRemoteDevice*, Solid::Ifaces::BluetoothRemoteDevice*> Solid::BluetoothInterfacePrivate::findRegisteredBluetoothRemoteDevice(const QString &ubi) const
{
    if (remoteDeviceMap.contains(ubi)) {
        return remoteDeviceMap[ubi];
    } else {
        Ifaces::BluetoothInterface *backend = qobject_cast<Ifaces::BluetoothInterface*>(backendObject());
        Ifaces::BluetoothRemoteDevice *iface = 0;

        if (backend != 0) {
            iface = qobject_cast<Ifaces::BluetoothRemoteDevice*>(backend->createBluetoothRemoteDevice(ubi));
        }

        if (iface != 0) {
            BluetoothRemoteDevice *device = new BluetoothRemoteDevice(iface);

            QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> pair(device, iface);
            remoteDeviceMap[ubi] = pair;

            return pair;
        } else {
            return QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*>(0, 0);
        }

    }
}

#include "bluetoothinterface.moc"
