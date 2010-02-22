/*  This file is part of the KDE project
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

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
#include "fakedevice.h"

#include "fakedeviceinterface.h"
#include "fakegenericinterface.h"
#include "fakeprocessor.h"
#include "fakeblock.h"
#include "fakestorage.h"
#include "fakecdrom.h"
#include "fakevolume.h"
#include "fakeopticaldisc.h"
#include "fakestorageaccess.h"
#include "fakecamera.h"
#include "fakeportablemediaplayer.h"
#include "fakenetworkinterface.h"
#include "fakeacadapter.h"
#include "fakebattery.h"
#include "fakebutton.h"
#include "fakeaudiointerface.h"
#include "fakedvbinterface.h"
#include "fakesmartcardreader.h"

#include <QtCore/QStringList>
#include <QtDBus/QDBusConnection>

#include <solid/genericinterface.h>

#include "fakedevice_p.h"

using namespace Solid::Backends::Fake;

FakeDevice::FakeDevice(const QString &udi, const QMap<QString, QVariant> &propertyMap)
    : Solid::Ifaces::Device(), d(new Private)
{
    d->udi = udi;
    d->propertyMap = propertyMap;
    d->interfaceList = d->propertyMap["interfaces"].toString().simplified().split(',');
    d->interfaceList << "GenericInterface";
    d->locked = false;
    d->broken = false;

    QDBusConnection::sessionBus().registerObject(udi, this, QDBusConnection::ExportNonScriptableSlots);

    // Force instantiation of all the device interfaces
    // this way they'll get exported on the bus
    // that means they'll be created twice, but that won't be
    // a problem for unit testing.
    foreach (const QString &interface, d->interfaceList)
    {
        Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(interface);
        createDeviceInterface(type);
    }

    connect(d.data(), SIGNAL(propertyChanged(const QMap<QString,int> &)),
            this, SIGNAL(propertyChanged(const QMap<QString,int> &)));
    connect(d.data(), SIGNAL(conditionRaised(const QString &, const QString &)),
            this, SIGNAL(conditionRaised(const QString &, const QString &)));
}

FakeDevice::FakeDevice(const FakeDevice& dev)
    : Solid::Ifaces::Device(), d(dev.d)
{
    connect(d.data(), SIGNAL(propertyChanged(const QMap<QString,int> &)),
            this, SIGNAL(propertyChanged(const QMap<QString,int> &)));
    connect(d.data(), SIGNAL(conditionRaised(const QString &, const QString &)),
            this, SIGNAL(conditionRaised(const QString &, const QString &)));
}

FakeDevice::~FakeDevice()
{
}

QString FakeDevice::udi() const
{
    return d->udi;
}

QString FakeDevice::parentUdi() const
{
    return d->propertyMap["parent"].toString();
}

QString FakeDevice::vendor() const
{
    return d->propertyMap["vendor"].toString();
}

QString FakeDevice::product() const
{
    return d->propertyMap["name"].toString();
}

QString FakeDevice::icon() const
{
    if(parentUdi().isEmpty()) {
        return "system";
    } else if (queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) {
        return "cdrom-unmount";
    } else if (queryDeviceInterface(Solid::DeviceInterface::PortableMediaPlayer)) {
        return "ipod-unmount";
    } else if (queryDeviceInterface(Solid::DeviceInterface::Camera)) {
        return "camera-unmount";
    } else if(queryDeviceInterface(Solid::DeviceInterface::Processor)) {
        return "cpu";
    } else if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive)) {
        return "hdd-unmount";
    } else if (queryDeviceInterface(Solid::DeviceInterface::Block)) {
        return "blockdevice";
    } else {
        return "hwinfo";
    }
}

QStringList FakeDevice::emblems() const
{
    QStringList res;

    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess)) {
        if (property("isMounted").toBool()) {
            res << "emblem-mounted";
        } else {
            res << "emblem-unmounted";
        }
    }

    return res;
}

QString FakeDevice::description() const
{
    return product();
}

QVariant FakeDevice::property(const QString &key) const
{
    return d->propertyMap[key];
}

QMap<QString, QVariant> FakeDevice::allProperties() const
{
    return d->propertyMap;
}

bool FakeDevice::propertyExists(const QString &key) const
{
    return d->propertyMap.contains(key);
}

bool FakeDevice::setProperty(const QString &key, const QVariant &value)
{
    if (d->broken) return false;

    Solid::GenericInterface::PropertyChange change_type = Solid::GenericInterface::PropertyModified;

    if (!d->propertyMap.contains(key))
    {
        change_type = Solid::GenericInterface::PropertyAdded;
    }

    d->propertyMap[key] = value;

    QMap<QString,int> change;
    change[key] = change_type;

    emit d->propertyChanged(change);

    return true;
}

bool FakeDevice::removeProperty(const QString &key)
{
    if (d->broken || !d->propertyMap.contains(key)) return false;

    d->propertyMap.remove(key);

    QMap<QString,int> change;
    change[key] = Solid::GenericInterface::PropertyRemoved;

    emit d->propertyChanged(change);

    return true;
}

void FakeDevice::setBroken(bool broken)
{
    d->broken = broken;
}

bool FakeDevice::isBroken()
{
    return d->broken;
}

bool FakeDevice::lock(const QString &reason)
{
    if (d->broken || d->locked) return false;

    d->locked = true;
    d->lockReason = reason;

    return true;
}

bool FakeDevice::unlock()
{
    if (d->broken || !d->locked) return false;

    d->locked = false;
    d->lockReason.clear();

    return true;
}

bool FakeDevice::isLocked() const
{
    return d->locked;
}

QString FakeDevice::lockReason() const
{
    return d->lockReason;
}

void FakeDevice::raiseCondition(const QString &condition, const QString &reason)
{
    emit d->conditionRaised(condition, reason);
}

bool FakeDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    return d->interfaceList.contains(Solid::DeviceInterface::typeToString(type));
}

QObject *FakeDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    // Do not try to cast with a unsupported device interface.
    if (!queryDeviceInterface(type))
        return 0;

    FakeDeviceInterface *iface = 0;

    switch(type)
    {
    case Solid::DeviceInterface::GenericInterface:
        iface = new FakeGenericInterface(this);
        break;
    case Solid::DeviceInterface::Processor:
        iface = new FakeProcessor(this);
        break;
    case Solid::DeviceInterface::Block:
        iface = new FakeBlock(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new FakeStorage(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new FakeCdrom(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new FakeVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new FakeOpticalDisc(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new FakeStorageAccess(this);
        break;
    case Solid::DeviceInterface::Camera:
        iface = new FakeCamera(this);
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        iface = new FakePortableMediaPlayer(this);
        break;
    case Solid::DeviceInterface::NetworkInterface:
        iface = new FakeNetworkInterface(this);
        break;
    case Solid::DeviceInterface::AcAdapter:
        iface = new FakeAcAdapter(this);
        break;
    case Solid::DeviceInterface::Battery:
        iface = new FakeBattery(this);
        break;
    case Solid::DeviceInterface::Button:
        iface = new FakeButton(this);
        break;
    case Solid::DeviceInterface::AudioInterface:
        iface = new FakeAudioInterface(this);
        break;
    case Solid::DeviceInterface::DvbInterface:
        iface = new FakeDvbInterface(this);
        break;
    case Solid::DeviceInterface::Video:
        break;
    case Solid::DeviceInterface::SmartCardReader:
        iface = new FakeSmartCardReader(this);
        break;
    case Solid::DeviceInterface::SerialInterface:
        break;
    case Solid::DeviceInterface::Unknown:
        break;
    case Solid::DeviceInterface::Last:
        break;
    }

    if(iface)
    {
        QDBusConnection::sessionBus().registerObject(d->udi+'/'+Solid::DeviceInterface::typeToString(type), iface,
                                                      QDBusConnection::ExportNonScriptableSlots);
    }

    return iface;
}

#include "backends/fakehw/fakedevice.moc"
#include "backends/fakehw/fakedevice_p.moc"
