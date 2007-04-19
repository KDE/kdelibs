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

#include <solid/experimental/ifaces/bluetoothinterface.h>

#include "frontendobject_p.h"

#include "../soliddefs_p.h"
#include "bluetoothmanager.h"
#include "bluetoothinterface.h"
#include "bluetoothremotedevice.h"

namespace SolidExperimental
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

SolidExperimental::BluetoothInterface::BluetoothInterface()
        : QObject(), d(new BluetoothInterfacePrivate(this))
{}

SolidExperimental::BluetoothInterface::BluetoothInterface(const QString &ubi)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    const BluetoothInterface &device = BluetoothManager::self().findBluetoothInterface(ubi);
    d->setBackendObject(device.d->backendObject());
}

SolidExperimental::BluetoothInterface::BluetoothInterface(QObject *backendObject)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    d->setBackendObject(backendObject);
}

SolidExperimental::BluetoothInterface::BluetoothInterface(const BluetoothInterface &device)
        : QObject(), d(new BluetoothInterfacePrivate(this))
{
    d->setBackendObject(device.d->backendObject());
}

SolidExperimental::BluetoothInterface::~BluetoothInterface()
{
    // Delete all the interfaces, they are now outdated
    typedef QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> BluetoothRemoteDeviceIfacePair;

    // Delete all the devices, they are now outdated
    foreach(const BluetoothRemoteDeviceIfacePair &pair, d->remoteDeviceMap.values()) {
        delete pair.first;
        delete pair.second;
    }
}

SolidExperimental::BluetoothInterface &SolidExperimental::BluetoothInterface::operator=(const SolidExperimental::BluetoothInterface & dev)
{
    d->setBackendObject(dev.d->backendObject());

    return *this;
}

QString SolidExperimental::BluetoothInterface::ubi() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), ubi());
}

const SolidExperimental::BluetoothRemoteDevice &SolidExperimental::BluetoothInterface::findBluetoothRemoteDevice(const QString &ubi) const
{
    QPair<BluetoothRemoteDevice*, Ifaces::BluetoothRemoteDevice*> pair = d->findRegisteredBluetoothRemoteDevice(ubi);

    if (pair.first != 0) {
        return *pair.first;
    } else {
        return d->invalidDevice;
    }
}

QString SolidExperimental::BluetoothInterface::address() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), address());
}

QString SolidExperimental::BluetoothInterface::version() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), version());
}

QString SolidExperimental::BluetoothInterface::revision() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), revision());
}

QString SolidExperimental::BluetoothInterface::manufacturer() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), manufacturer());
}

QString SolidExperimental::BluetoothInterface::company() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), company());
}

SolidExperimental::BluetoothInterface::Mode SolidExperimental::BluetoothInterface::mode() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), SolidExperimental::BluetoothInterface::Off, mode());
}

int SolidExperimental::BluetoothInterface::discoverableTimeout() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), 0, discoverableTimeout());
}

bool SolidExperimental::BluetoothInterface::isDiscoverable() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), false, isDiscoverable());
}

SolidExperimental::BluetoothRemoteDeviceList SolidExperimental::BluetoothInterface::listConnections() const
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

QString SolidExperimental::BluetoothInterface::majorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), majorClass());
}

QStringList SolidExperimental::BluetoothInterface::listAvailableMinorClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(), listAvailableMinorClasses());
}

QString SolidExperimental::BluetoothInterface::minorClass() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), minorClass());
}

QStringList SolidExperimental::BluetoothInterface::serviceClasses() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(), serviceClasses());
}

QString SolidExperimental::BluetoothInterface::name() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QString(), name());
}

QStringList SolidExperimental::BluetoothInterface::listBondings() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(), listBondings());
}

bool SolidExperimental::BluetoothInterface::isPeriodicDiscoveryActive() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), false, isPeriodicDiscoveryActive());
}

bool SolidExperimental::BluetoothInterface::isPeriodicDiscoveryNameResolvingActive() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), false,
                      isPeriodicDiscoveryNameResolvingActive());
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList SolidExperimental::BluetoothInterface::listRemoteDevices() const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(),
                      listRemoteDevices());
}

// TODO: QStringList or BluetoothRemoteDeviceList?
QStringList SolidExperimental::BluetoothInterface::listRecentRemoteDevices(const QDateTime &date) const
{
    return_SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), QStringList(),
                      listRecentRemoteDevices(date));
}

/***************************************************************/

void SolidExperimental::BluetoothInterface::setMode(const SolidExperimental::BluetoothInterface::Mode mode)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setMode(mode));
}

void SolidExperimental::BluetoothInterface::setDiscoverableTimeout(int timeout)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setDiscoverableTimeout(timeout));
}

void SolidExperimental::BluetoothInterface::setMinorClass(const QString &minor)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setMinorClass(minor));
}

void SolidExperimental::BluetoothInterface::setName(const QString &name)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setName(name));
}

void SolidExperimental::BluetoothInterface::discoverDevices()
{
    kDebug() << k_funcinfo << endl;
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), discoverDevices());
}

void SolidExperimental::BluetoothInterface::discoverDevicesWithoutNameResolving()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), discoverDevicesWithoutNameResolving());
}

void SolidExperimental::BluetoothInterface::cancelDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), cancelDiscovery());
}

void SolidExperimental::BluetoothInterface::startPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), startPeriodicDiscovery());
}

void SolidExperimental::BluetoothInterface::stopPeriodicDiscovery()
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), stopPeriodicDiscovery());
}

void SolidExperimental::BluetoothInterface::setPeriodicDiscoveryNameResolving(bool resolveNames)
{
    SOLID_CALL(Ifaces::BluetoothInterface*, d->backendObject(), setPeriodicDiscoveryNameResolving(resolveNames));
}

void SolidExperimental::BluetoothInterfacePrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

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

QPair<SolidExperimental::BluetoothRemoteDevice*, SolidExperimental::Ifaces::BluetoothRemoteDevice*> SolidExperimental::BluetoothInterfacePrivate::findRegisteredBluetoothRemoteDevice(const QString &ubi) const
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
