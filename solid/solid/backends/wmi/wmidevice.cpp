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

#include <QtCore/QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusMetaType>

#include <solid/genericinterface.h>

#include "wmidevice.h"
#include "wmideviceinterface.h"
#include "wmigenericinterface.h"
#include "wmiprocessor.h"
#include "wmiblock.h"
#include "wmistorageaccess.h"
#include "wmistorage.h"
#include "wmicdrom.h"
#include "wmivolume.h"
#include "wmiopticaldisc.h"
#include "wmicamera.h"
#include "wmiportablemediaplayer.h"
#include "wminetworkinterface.h"
#include "wmiacadapter.h"
#include "wmibattery.h"
#include "wmibutton.h"
#include "wmiaudiointerface.h"
#include "wmidvbinterface.h"
#include "wmivideo.h"

using namespace Solid::Backends::Wmi;

class Solid::Backends::Wmi::WmiDevicePrivate
{
public:
    WmiDevicePrivate(const QString &udi)
        : device("org.freedesktop.Wmi",
                  udi,
                  "org.freedesktop.Wmi.Device",
                  QDBusConnection::systemBus()),
          cacheSynced(false), parent(0) { }

    QDBusInterface device;
    QMap<QString,QVariant> cache;
    bool cacheSynced;
    WmiDevice *parent;
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

WmiDevice::WmiDevice(const QString &udi)
    : Device(), d(new WmiDevicePrivate(udi))
{
    qDBusRegisterMetaType<ChangeDescription>();
    qDBusRegisterMetaType< QList<ChangeDescription> >();

    d->device.connection().connect("org.freedesktop.Wmi",
                                    udi, "org.freedesktop.Wmi.Device",
                                    "PropertyModified",
                                    this, SLOT(slotPropertyModified(int, const QList<ChangeDescription> &)));
    d->device.connection().connect("org.freedesktop.Wmi",
                                    udi, "org.freedesktop.Wmi.Device",
                                    "Condition",
                                    this, SLOT(slotCondition(const QString &, const QString &)));
}

WmiDevice::~WmiDevice()
{
    delete d->parent;
    delete d;
}

QString WmiDevice::udi() const
{
    return property("info.udi").toString();
}

QString WmiDevice::parentUdi() const
{
    return property("info.parent").toString();
}

QString WmiDevice::vendor() const
{
    return property("info.vendor").toString();
}

QString WmiDevice::product() const
{
    return property("info.product").toString();
}

QString WmiDevice::icon() const
{
    QString category = property("info.category").toString();

    if(parentUdi().isEmpty()) {

        QString formfactor = property("system.formfactor").toString();
        if (formfactor=="laptop") {
            return "computer-laptop";
        } else {
            return "computer";
        }

    } else if (category=="storage") {

        if (property("storage.drive_type").toString()=="floppy") {
            return "media-floppy";
        } else if (property("storage.drive_type").toString()=="cdrom") {
            return "drive-optical";
        } else if (property("storage.hotpluggable").toBool()) {
            if (property("storage.bus").toString()=="usb") {
                if (property("storage.no_partitions_hint").toBool()
                 || property("storage.removable.media_size").toLongLong()<4000000000LL) {
                    return "drive-removable-media-usb-pendrive";
                } else {
                    return "drive-removable-media-usb";
                }
            }

            return "drive-removable-media";
        }

        return "drive-harddisk";

    } else if (category=="volume") {

        QStringList capabilities = property("info.capabilities").toStringList();

        if (capabilities.contains("volume.disc")) {
            bool has_video = property("volume.disc.is_vcd").toBool()
                          || property("volume.disc.is_svcd").toBool()
                          || property("volume.disc.is_videodvd").toBool();
            bool has_audio = property("volume.disc.has_audio").toBool();
            bool recordable = property("volume.disc.is_blank").toBool()
                          || property("volume.disc.is_appendable").toBool()
                          || property("volume.disc.is_rewritable").toBool();

            if (has_video) {
                return "media-optical-video";
            } else if (has_audio) {
                return "media-optical-audio";
            } else if (recordable) {
                return "media-optical-recordable";
            } else {
                return "media-optical";
            }

        } else {
            if (!d->parent) {
                d->parent = new WmiDevice(parentUdi());
            }
            QString iconName = d->parent->icon();

            if (!iconName.isEmpty()) {
                return iconName;
            }

            return "drive-harddisk";
        }

    } else if (category=="camera") {
        return "camera-photo";

    } else if (category=="input") {
        QStringList capabilities = property("info.capabilities").toStringList();

        if (capabilities.contains("input.mouse")) {
            return "input-mouse";
        } else if (capabilities.contains("input.keyboard")) {
            return "input-keyboard";
        } else if (capabilities.contains("input.joystick")) {
            return "input-gaming";
        } else if (capabilities.contains("input.tablet")) {
            return "input-tablet";
        }

    } else if (category=="portable_audio_player") {
        QStringList protocols = property("portable_audio_player.access_method.protocols").toStringList();

        if (protocols.contains("ipod")) {
            return "multimedia-player-apple-ipod";
        } else {
            return "multimedia-player";
        }
    } else if (category=="battery") {
        return "battery";
    } else if (category=="processor") {
        return "cpu"; // FIXME: Doesn't follow icon spec
    } else if (category=="video4linux") {
        return "camera-web";
    }

    return QString();
}

QVariant WmiDevice::property(const QString &key) const
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
      && reply.errorName()!="org.freedesktop.Wmi.NoSuchProperty")
    {
        qWarning() << Q_FUNC_INFO << " error: " << reply.errorName()
                   << ", " << reply.arguments().at(0).toString() << endl;
        return QVariant();
    }

    if (reply.errorName()=="org.freedesktop.Wmi.NoSuchProperty")
    {
        d->cache[key] = QVariant();
    }
    else
    {
        d->cache[key] = reply.arguments().at(0);
    }

    return d->cache[key];
}

QMap<QString, QVariant> WmiDevice::allProperties() const
{
    if (d->cacheSynced)
    {
        return d->cache;
    }

    QDBusReply<QVariantMap> reply = d->device.call("GetAllProperties");

    if (!reply.isValid())
    {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name()
                   << ", " << reply.error().message() << endl;
        return QVariantMap();
    }

    d->cache = reply;
    d->cacheSynced = true;

    return reply;
}

bool WmiDevice::propertyExists(const QString &key) const
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
        qDebug() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        return false;
    }

    return reply;
}

bool WmiDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    // Special cases not matching with WMI capabilities
    if (type==Solid::DeviceInterface::GenericInterface) {
        return true;
    } else if (type==Solid::DeviceInterface::StorageAccess) {
        return property("info.interfaces").toStringList().contains("org.freedesktop.Wmi.Device.Volume")
            || property("info.interfaces").toStringList().contains("org.freedesktop.Wmi.Device.Volume.Crypto");
    }
    else if (type==Solid::DeviceInterface::Video) {
        if (!property("video4linux.device").toString().contains("video" ) )
          return false;
    }

    QStringList cap_list = DeviceInterface::toStringList(type);
    QStringList result;

    foreach (const QString &cap, cap_list)
    {
        QDBusReply<bool> reply = d->device.call("QueryCapability", cap);

        if (!reply.isValid())
        {
            qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
            return false;
        }

        if (reply) return reply;
    }

    return false;
}

QObject *WmiDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    if (!queryDeviceInterface(type)) {
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
    case Solid::DeviceInterface::StorageAccess:
        iface = new StorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new Storage(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new Cdrom(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
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
    case Solid::DeviceInterface::NetworkInterface:
        iface = new NetworkInterface(this);
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
    case Solid::DeviceInterface::AudioInterface:
        iface = new AudioInterface(this);
        break;
    case Solid::DeviceInterface::DvbInterface:
        iface = new DvbInterface(this);
        break;
    case Solid::DeviceInterface::Video:
        iface = new Video(this);
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
        break;
    }

    return iface;
}

void WmiDevice::slotPropertyModified(int /*count */, const QList<ChangeDescription> &changes)
{
    QMap<QString,int> result;

    foreach (const ChangeDescription &change, changes)
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

void WmiDevice::slotCondition(const QString &condition, const QString &reason)
{
    emit conditionRaised(condition, reason);
}

#include "backends/wmi/wmidevice.moc"
