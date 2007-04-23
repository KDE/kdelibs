/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Kevin Ottens <ervin@kde.org>

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

#include <kdebug.h>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMetaType>

#include "haldevice.h"
#include "deviceinterface.h"
#include "genericinterface.h"
#include "processor.h"
#include "block.h"
#include "storage.h"
#include "cdrom.h"
#include "volume.h"
#include "opticaldisc.h"
#include "camera.h"
#include "portablemediaplayer.h"
#include "networkhw.h"
#include "acadapter.h"
#include "battery.h"
#include "button.h"
#include "display.h"
#include "audiohw.h"
#include "dvbhw.h"

class HalDevicePrivate
{
public:
    HalDevicePrivate(const QString &udi)
        : device("org.freedesktop.Hal",
                  udi,
                  "org.freedesktop.Hal.Device",
                  QDBusConnection::systemBus()),
          cacheSynced(false) { }

    QDBusInterface device;
    QMap<QString,QVariant> cache;
    bool cacheSynced;
};

Q_DECLARE_METATYPE(ChangeDescription)
Q_DECLARE_METATYPE(QList<ChangeDescription>)

const QDBusArgument &operator<<(QDBusArgument &arg, const ChangeDescription &change)
{
    arg.beginStructure();
    arg << change.key << change.added << change.removed;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ChangeDescription &change)
{
    arg.beginStructure();
    arg >> change.key >> change.added >> change.removed;
    arg.endStructure();
    return arg;
}

HalDevice::HalDevice(const QString &udi)
    : Device(), d(new HalDevicePrivate(udi))
{
    qDBusRegisterMetaType<ChangeDescription>();
    qDBusRegisterMetaType< QList<ChangeDescription> >();

    d->device.connection().connect("org.freedesktop.Hal",
                                    udi, "org.freedesktop.Hal.Device",
                                    "PropertyModified",
                                    this, SLOT(slotPropertyModified(int, const QList<ChangeDescription> &)));
    d->device.connection().connect("org.freedesktop.Hal",
                                    udi, "org.freedesktop.Hal.Device",
                                    "Condition",
                                    this, SLOT(slotCondition(const QString &, const QString &)));
}

HalDevice::~HalDevice()
{
    delete d;
}

QString HalDevice::udi() const
{
    return property("info.udi").toString();
}

QString HalDevice::parentUdi() const
{
    return property("info.parent").toString();
}

QString HalDevice::vendor() const
{
    return property("info.vendor").toString();
}

QString HalDevice::product() const
{
    return property("info.product").toString();
}

bool HalDevice::setProperty(const QString &key, const QVariant &value)
{
    QList<QVariant> args;
    args << key << value;
    QDBusReply<void> reply = d->device.callWithArgumentList(QDBus::BlockWithGui,
                                                             "SetProperty", args);

    if (!reply.isValid())
    {
        kDebug() << k_funcinfo << " error: " << reply.error().name() << endl;
        return false;
    }

    d->cache[key] = value;

    return true;
}

QVariant HalDevice::property(const QString &key) const
{
    if (d->cache.contains(key))
    {
        return d->cache[key];
    }
    else if (d->cacheSynced)
    {
        return QVariant();
    }

    QDBusMessage reply = d->device.call("GetProperty", key);

    if (reply.type()!=QDBusMessage::ReplyMessage
      && reply.errorName()!="org.freedesktop.Hal.NoSuchProperty")
    {
        kDebug() << k_funcinfo << " error: " << reply.errorName()
                 << ", " << reply.arguments().at(0).toString() << endl;
        return QVariant();
    }

    if (reply.errorName()=="org.freedesktop.Hal.NoSuchProperty")
    {
        d->cache[key] = QVariant();
    }
    else
    {
        d->cache[key] = reply.arguments().at(0);
    }

    return d->cache[key];
}

QMap<QString, QVariant> HalDevice::allProperties() const
{
    if (d->cacheSynced)
    {
        return d->cache;
    }

    QDBusReply<QVariantMap> reply = d->device.call("GetAllProperties");

    if (!reply.isValid())
    {
        kDebug() << k_funcinfo << " error: " << reply.error().name()
                 << ", " << reply.error().message() << endl;
        return QVariantMap();
    }

    d->cache = reply;
    d->cacheSynced = true;

    return reply;
}

bool HalDevice::removeProperty(const QString &key)
{
    QList<QVariant> args;
    args << key;
    QDBusReply<void> reply = d->device.callWithArgumentList(QDBus::BlockWithGui,
                                                             "RemoveProperty", args);

    if (!reply.isValid())
    {
        kDebug() << k_funcinfo << " error: " << reply.error().name() << endl;
        return false;
    }

    d->cache.remove(key);

    return true;
}

bool HalDevice::propertyExists(const QString &key) const
{
    if (d->cache.contains(key))
    {
        return d->cache[key].isValid();
    }
    else if (d->cacheSynced)
    {
        return false;
    }

    QDBusReply<bool> reply = d->device.call("PropertyExists", key);

    if (!reply.isValid())
    {
        kDebug() << k_funcinfo << " error: " << reply.error().name() << endl;
        return false;
    }

    return reply;
}

bool HalDevice::addDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    QList<QVariant> args;
    args << DeviceInterface::toStringList(type).first();
    QDBusReply<void> reply = d->device.callWithArgumentList(QDBus::BlockWithGui,
                                                             "AddDeviceInterface", args);

    if (!reply.isValid())
    {
        kDebug() << k_funcinfo << " error: " << reply.error().name() << endl;
        return false;
    }

    return true;
}

bool HalDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    QStringList cap_list = DeviceInterface::toStringList(type);
    QStringList result;

    foreach (QString cap, cap_list)
    {
        QDBusReply<bool> reply = d->device.call("QueryCapability", cap);

        if (!reply.isValid())
        {
            kDebug() << k_funcinfo << " error: " << reply.error().name() << endl;
            return false;
        }

        if (reply) return reply;
    }

    return false;
}

QObject *HalDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (type!=Solid::DeviceInterface::GenericInterface
     && !queryDeviceInterface(type)) {
        return 0;
    }

    DeviceInterface *iface = 0;

    switch (type)
    {
    case Solid::DeviceInterface::GenericInterface:
        iface = new GenericInterface(this);
        break;
    case Solid::DeviceInterface::Processor:
        iface = new Processor(this);
        break;
    case Solid::DeviceInterface::Block:
        iface = new Block(this);
        break;
    case Solid::DeviceInterface::Storage:
        iface = new Storage(this);
        break;
    case Solid::DeviceInterface::Cdrom:
        iface = new Cdrom(this);
        break;
    case Solid::DeviceInterface::Volume:
        iface = new Volume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new OpticalDisc(this);
        break;
    case Solid::DeviceInterface::Camera:
        iface = new Camera(this);
        break;
    case Solid::DeviceInterface::PortableMediaPlayer:
        iface = new PortableMediaPlayer(this);
        break;
    case Solid::DeviceInterface::NetworkHw:
        iface = new NetworkHw(this);
        break;
    case Solid::DeviceInterface::AcAdapter:
        iface = new AcAdapter(this);
        break;
    case Solid::DeviceInterface::Battery:
        iface = new Battery(this);
        break;
    case Solid::DeviceInterface::Button:
        iface = new Button(this);
        break;
    case Solid::DeviceInterface::Display:
        iface = new Display(this);
        break;
    case Solid::DeviceInterface::AudioHw:
        iface = new AudioHw(this);
        break;
    case Solid::DeviceInterface::DvbHw:
        iface = new DvbHw(this);
        break;
    case Solid::DeviceInterface::Unknown:
        break;
    }

    return iface;
}

bool HalDevice::lock(const QString &reason)
{
    QList<QVariant> args;
    args << reason;
    QDBusReply<void> reply = d->device.callWithArgumentList(QDBus::BlockWithGui,
                                                             "Lock", args);

    if (!reply.isValid())
    {
        kDebug() << k_funcinfo << " error: " << reply.error().name() << endl;
        return false;
    }

    return true;
}

bool HalDevice::unlock()
{
    QDBusReply<void> reply = d->device.callWithArgumentList(QDBus::AutoDetect,
                                                             "Unlock", QList<QVariant>());

    if (!reply.isValid())
    {
        kDebug() << k_funcinfo << " error: " << reply.error().name() << endl;
        return false;
    }

    return true;
}

bool HalDevice::isLocked() const
{
    return property("info.locked").toBool();
}

QString HalDevice::lockReason() const
{
    return property("info.locked.reason").toString();
}

void HalDevice::slotPropertyModified(int /*count */, const QList<ChangeDescription> &changes)
{
    QMap<QString,int> result;

    foreach (const ChangeDescription change, changes)
    {
        QString key = change.key;
        bool added = change.added;
        bool removed = change.removed;

        Solid::GenericInterface::PropertyChange type = Solid::GenericInterface::PropertyModified;

        if (added)
        {
            type = Solid::GenericInterface::PropertyAdded;
        }
        else if (removed)
        {
            type = Solid::GenericInterface::PropertyRemoved;
        }

        result[key] = type;

        d->cache.remove(key);
    }

    d->cacheSynced = false;

    emit propertyChanged(result);
}

void HalDevice::slotCondition(const QString &condition, const QString &reason)
{
    emit conditionRaised(condition, reason);
}

#include "haldevice.moc"
