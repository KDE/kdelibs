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

#include "haldevice.h"
#include "haldeviceinterface.h"
#include "halgenericinterface.h"
#include "halprocessor.h"
#include "halblock.h"
#include "halstorageaccess.h"
#include "halstorage.h"
#include "halcdrom.h"
#include "halvolume.h"
#include "halopticaldisc.h"
#include "halcamera.h"
#include "halportablemediaplayer.h"
#include "halnetworkinterface.h"
#include "halacadapter.h"
#include "halbattery.h"
#include "halbutton.h"
#include "halaudiointerface.h"
#include "haldvbinterface.h"
#include "halvideo.h"
#include "halserialinterface.h"

using namespace Solid::Backends::Hal;

class Solid::Backends::Hal::HalDevicePrivate
{
public:
    HalDevicePrivate(const QString &udi)
        : device("org.freedesktop.Hal",
                  udi,
                  "org.freedesktop.Hal.Device",
                  QDBusConnection::systemBus()),
          cacheSynced(false), parent(0) { }

    QDBusInterface device;
    QMap<QString,QVariant> cache;
    bool cacheSynced;
    HalDevice *parent;
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
    delete d->parent;
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

QString HalDevice::icon() const
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
                d->parent = new HalDevice(parentUdi());
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
    } else if (category == "alsa" || category == "oss") {
        // Sorry about this const_cast, but it's the best way to not copy the code from
        // AudioInterface.
        const Hal::AudioInterface audioIface(const_cast<HalDevice *>(this));
        switch (audioIface.soundcardType()) {
        case Solid::AudioInterface::InternalSoundcard:
            return QLatin1String("audio-card");
        case Solid::AudioInterface::UsbSoundcard:
            return QLatin1String("audio-card-usb");
        case Solid::AudioInterface::FirewireSoundcard:
            return QLatin1String("audio-card-firewire");
        case Solid::AudioInterface::Headset:
            if (udi().contains("usb", Qt::CaseInsensitive) ||
                    audioIface.name().contains("usb", Qt::CaseInsensitive)) {
                return QLatin1String("audio-headset-usb");
            } else {
                return QLatin1String("audio-headset");
            }
        case Solid::AudioInterface::Modem:
            return QLatin1String("modem");
        }
    } else if (category == "serial") {
        // TODO - a serial device can be a modem, or just
        // a COM port - need a new icon?
        return QLatin1String("modem");
    }

    return QString();
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
        qWarning() << Q_FUNC_INFO << " error: " << reply.errorName()
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
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name()
                   << ", " << reply.error().message() << endl;
        return QVariantMap();
    }

    d->cache = reply;
    d->cacheSynced = true;

    return reply;
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
        qDebug() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        return false;
    }

    return reply;
}

bool HalDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    // Special cases not matching with HAL capabilities
    if (type==Solid::DeviceInterface::GenericInterface) {
        return true;
    } else if (type==Solid::DeviceInterface::StorageAccess) {
        return property("info.interfaces").toStringList().contains("org.freedesktop.Hal.Device.Volume")
            || property("info.interfaces").toStringList().contains("org.freedesktop.Hal.Device.Volume.Crypto");
    }
    else if (type==Solid::DeviceInterface::Video) {
        if (!property("video4linux.device").toString().contains("video" ) )
          return false;
    }

    QStringList cap_list = DeviceInterface::toStringList(type);

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

QObject *HalDevice::createDeviceInterface(const Solid::DeviceInterface::Type &type)
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
    case Solid::DeviceInterface::SerialInterface:
        iface = new SerialInterface(this);
        break;
    case Solid::DeviceInterface::Unknown:
    case Solid::DeviceInterface::Last:
        break;
    }

    return iface;
}

void HalDevice::slotPropertyModified(int /*count */, const QList<ChangeDescription> &changes)
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

void HalDevice::slotCondition(const QString &condition, const QString &reason)
{
    emit conditionRaised(condition, reason);
}

#include "backends/hal/haldevice.moc"
