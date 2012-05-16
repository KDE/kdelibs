/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010-2012 Lukáš Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "udisksdevice.h"
#include "udisksblock.h"
#include "udisksdeviceinterface.h"
#include "udisksstoragevolume.h"
#include "udisksopticaldisc.h"
#include "udisksopticaldrive.h"
#include "udisksstorageaccess.h"
#include "udisksgenericinterface.h"

#include <solid/genericinterface.h>
#include <solid/deviceinterface.h>
#include <solid/device.h>

#include <QtCore/QDebug>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusArgument>

#include <QtXml/QDomDocument>

using namespace Solid::Backends::UDisks2;

// Adapted from KLocale as Solid needs to be Qt-only
static QString formatByteSize(double size)
{
    // Per IEC 60027-2

    // Binary prefixes
    //Tebi-byte             TiB             2^40    1,099,511,627,776 bytes
    //Gibi-byte             GiB             2^30    1,073,741,824 bytes
    //Mebi-byte             MiB             2^20    1,048,576 bytes
    //Kibi-byte             KiB             2^10    1,024 bytes

    QString s;
    // Gibi-byte
    if ( size >= 1073741824.0 )
    {
        size /= 1073741824.0;
        if ( size > 1024 ) // Tebi-byte
            s = QObject::tr("%1 TiB").arg(QLocale().toString(size / 1024.0, 'f', 1));
        else
            s = QObject::tr("%1 GiB").arg(QLocale().toString(size, 'f', 1));
    }
    // Mebi-byte
    else if ( size >= 1048576.0 )
    {
        size /= 1048576.0;
        s = QObject::tr("%1 MiB").arg(QLocale().toString(size, 'f', 1));
    }
    // Kibi-byte
    else if ( size >= 1024.0 )
    {
        size /= 1024.0;
        s = QObject::tr("%1 KiB").arg(QLocale().toString(size, 'f', 1));
    }
    // Just byte
    else if ( size > 0 )
    {
        s = QObject::tr("%1 B").arg(QLocale().toString(size, 'f', 1));
    }
    // Nothing
    else
    {
        s = QObject::tr("0 B");
    }
    return s;
}

Device::Device(const QString &udi)
    : Solid::Ifaces::Device()
    , m_udi(udi)
{
    m_device = new QDBusInterface(UD2_DBUS_SERVICE, m_udi,
                                  QString(), // no interface, we aggregate them
                                  QDBusConnection::systemBus());

    if (m_device->isValid()) {
        QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE, m_udi, DBUS_INTERFACE_PROPS, "PropertiesChanged", this,
                                             SLOT(slotPropertiesChanged(QString,QVariantMap,QStringList)));

        QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE, UD2_DBUS_PATH, DBUS_INTERFACE_MANAGER, "InterfacesAdded",
                                             this, SLOT(slotInterfacesAdded(QDBusObjectPath,QVariantMapMap)));
        QDBusConnection::systemBus().connect(UD2_DBUS_SERVICE, UD2_DBUS_PATH, DBUS_INTERFACE_MANAGER, "InterfacesRemoved",
                                             this, SLOT(slotInterfacesRemoved(QDBusObjectPath,QStringList)));

        initInterfaces();
    }
}

Device::~Device()
{
    delete m_device;
}

QObject* Device::createDeviceInterface(const Solid::DeviceInterface::Type& type)
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
    case Solid::DeviceInterface::Block:
        iface = new Block(this);
        break;
    case Solid::DeviceInterface::StorageAccess:
        iface = new StorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new StorageDrive(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new OpticalDrive(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new StorageVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new OpticalDisc(this);
        break;
    default:
        break;
    }
    return iface;
}

bool Device::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
    switch (type) {
    case Solid::DeviceInterface::GenericInterface:
        return true;
    case Solid::DeviceInterface::Block:
        return isBlock();
    case Solid::DeviceInterface::StorageVolume:
        return isStorageVolume();
    case Solid::DeviceInterface::StorageAccess:
        return isStorageAccess();
    case Solid::DeviceInterface::StorageDrive:
        return isDrive();
    case Solid::DeviceInterface::OpticalDrive:
        return isOpticalDrive();
    case Solid::DeviceInterface::OpticalDisc:
        return isOpticalDisc();
    default:
        return false;
    }
}

QStringList Device::emblems() const
{
    QStringList res;

    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess))
    {
        const UDisks2::StorageAccess accessIface(const_cast<Device *>(this));
        if (accessIface.isAccessible())
        {
            if (isEncryptedContainer())
                res << "emblem-encrypted-unlocked";
            else
                res << "emblem-mounted";
        }
        else
        {
            if (isEncryptedContainer())
                res << "emblem-encrypted-locked";
            else
                res << "emblem-unmounted";
        }
    }

    return res;
}

QString Device::description() const
{
    const QString hintName = prop("HintName").toString();
    if (!hintName.isEmpty())
        return hintName;

    if (queryDeviceInterface(Solid::DeviceInterface::StorageDrive))
        return storageDescription();
    else if (queryDeviceInterface(Solid::DeviceInterface::StorageVolume))
        return volumeDescription();
    else
        return product();
}

QString Device::storageDescription() const
{
    QString description;
    const UDisks2::StorageDrive storageDrive(const_cast<Device*>(this));
    Solid::StorageDrive::DriveType drive_type = storageDrive.driveType();
    bool drive_is_hotpluggable = storageDrive.isHotpluggable();

    if (drive_type == Solid::StorageDrive::CdromDrive)
    {
        const UDisks2::OpticalDrive opticalDrive(const_cast<Device*>(this));
        Solid::OpticalDrive::MediumTypes mediumTypes = opticalDrive.supportedMedia();
        QString first;
        QString second;

        first = QObject::tr("CD-ROM", "First item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Cdr)
            first = QObject::tr("CD-R", "First item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Cdrw)
            first = QObject::tr("CD-RW", "First item of %1%2 Drive sentence");

        if (mediumTypes & Solid::OpticalDrive::Dvd)
            second = QObject::tr("/DVD-ROM", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Dvdplusr)
            second = QObject::tr("/DVD+R", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Dvdplusrw)
            second = QObject::tr("/DVD+RW", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Dvdr)
            second = QObject::tr("/DVD-R", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Dvdrw)
            second = QObject::tr("/DVD-RW", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Dvdram)
            second = QObject::tr("/DVD-RAM", "Second item of %1%2 Drive sentence");
        if ((mediumTypes & Solid::OpticalDrive::Dvdr) && (mediumTypes & Solid::OpticalDrive::Dvdplusr))
        {
            if(mediumTypes & Solid::OpticalDrive::Dvdplusdl)
                second = QObject::trUtf8("/DVD±R DL", "Second item of %1%2 Drive sentence");
            else
                second = QObject::trUtf8("/DVD±R", "Second item of %1%2 Drive sentence");
        }
        if ((mediumTypes & Solid::OpticalDrive::Dvdrw) && (mediumTypes & Solid::OpticalDrive::Dvdplusrw))
        {
            if((mediumTypes & Solid::OpticalDrive::Dvdplusdl) || (mediumTypes & Solid::OpticalDrive::Dvdplusdlrw))
                second = QObject::trUtf8("/DVD±RW DL", "Second item of %1%2 Drive sentence");
            else
                second = QObject::trUtf8("/DVD±RW", "Second item of %1%2 Drive sentence");
        }
        if (mediumTypes & Solid::OpticalDrive::Bd)
            second = QObject::tr("/BD-ROM", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Bdr)
            second = QObject::tr("/BD-R", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::Bdre)
            second = QObject::tr("/BD-RE", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::HdDvd)
            second = QObject::tr("/HD DVD-ROM", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::HdDvdr)
            second = QObject::tr("/HD DVD-R", "Second item of %1%2 Drive sentence");
        if (mediumTypes & Solid::OpticalDrive::HdDvdrw)
            second = QObject::tr("/HD DVD-RW", "Second item of %1%2 Drive sentence");

        if (drive_is_hotpluggable)
            description = QObject::tr("External %1%2 Drive", "%1 is CD-ROM/CD-R/etc; %2 is '/DVD-ROM'/'/DVD-R'/etc (with leading slash)").arg(first).arg(second);
        else
            description = QObject::tr("%1%2 Drive", "%1 is CD-ROM/CD-R/etc; %2 is '/DVD-ROM'/'/DVD-R'/etc (with leading slash)").arg(first).arg(second);

        return description;
    }

    if (drive_type == Solid::StorageDrive::Floppy)
    {
        if (drive_is_hotpluggable)
            description = QObject::tr("External Floppy Drive");
        else
            description = QObject::tr("Floppy Drive");

        return description;
    }

    bool drive_is_removable = storageDrive.isRemovable();

    if (drive_type == Solid::StorageDrive::HardDisk && !drive_is_removable)
    {
        QString size_str = formatByteSize(storageDrive.size());
        if (!size_str.isEmpty())
        {
            if (drive_is_hotpluggable)
                description = QObject::tr("%1 External Hard Drive", "%1 is the size").arg(size_str);
            else
                description = QObject::tr("%1 Hard Drive", "%1 is the size").arg(size_str);
        } else {
            if (drive_is_hotpluggable)
                description = QObject::tr("External Hard Drive");
            else
                description = QObject::tr("Hard Drive");
        }

        return description;
    }

    QString vendormodel_str;
    QString model = product();
    QString vendor_str = vendor();

    if (vendor_str.isEmpty())
    {
        if (!model.isEmpty())
            vendormodel_str = model;
    }
    else
    {
        if (model.isEmpty())
            vendormodel_str = vendor_str;
        else
        {
            if (model.startsWith(vendor_str))
            {
                // e.g. vendor is "Nokia" and model is "Nokia N950" we do not want "Nokia Nokia N950" as description
                vendormodel_str = model;
            }
            else
            {
                vendormodel_str = QObject::tr("%1 %2", "%1 is the vendor, %2 is the model of the device").arg(vendor_str).arg(model);
            }
        }
    }

    if (vendormodel_str.isEmpty())
        description = QObject::tr("Drive");
    else
        description = vendormodel_str;

    return description;
}

QString Device::volumeDescription() const
{
    QString description;
    const UDisks2::StorageVolume storageVolume(const_cast<Device*>(this));
    QString volume_label = prop("IdLabel").toString();
    if (volume_label.isEmpty())
        volume_label = prop("Name").toString();
    if (!volume_label.isEmpty())
        return volume_label;

    const QString drivePath = prop("Drive").value<QDBusObjectPath>().path();
    UDisks2::Device storageDevice(drivePath);
    const UDisks2::StorageDrive storageDrive(&storageDevice);
    Solid::StorageDrive::DriveType drive_type = storageDrive.driveType();

    // Handle media in optical drives
    if (drive_type == Solid::StorageDrive::CdromDrive)
    {
        const UDisks2::OpticalDisc disc(const_cast<Device*>(this));
        switch (disc.discType())
        {
            case Solid::OpticalDisc::UnknownDiscType:
            case Solid::OpticalDisc::CdRom:
                description = QObject::tr("CD-ROM");
                break;

            case Solid::OpticalDisc::CdRecordable:
                if (disc.isBlank())
                    description = QObject::tr("Blank CD-R");
                else
                    description = QObject::tr("CD-R");
                break;

            case Solid::OpticalDisc::CdRewritable:
                if (disc.isBlank())
                    description = QObject::tr("Blank CD-RW");
                else
                    description = QObject::tr("CD-RW");
                break;

            case Solid::OpticalDisc::DvdRom:
                description = QObject::tr("DVD-ROM");
                break;

            case Solid::OpticalDisc::DvdRam:
                if (disc.isBlank())
                    description = QObject::tr("Blank DVD-RAM");
                else
                    description = QObject::tr("DVD-RAM");
                break;

            case Solid::OpticalDisc::DvdRecordable:
                if (disc.isBlank())
                    description = QObject::tr("Blank DVD-R");
                else
                    description = QObject::tr("DVD-R");
                break;

            case Solid::OpticalDisc::DvdPlusRecordableDuallayer:
                if (disc.isBlank())
                    description = QObject::tr("Blank DVD+R Dual-Layer");
                else
                    description = QObject::tr("DVD+R Dual-Layer");
                break;

            case Solid::OpticalDisc::DvdRewritable:
                if (disc.isBlank())
                    description = QObject::tr("Blank DVD-RW");
                else
                    description = QObject::tr("DVD-RW");
                break;

            case Solid::OpticalDisc::DvdPlusRecordable:
                if (disc.isBlank())
                    description = QObject::tr("Blank DVD+R");
                else
                    description = QObject::tr("DVD+R");
                break;

            case Solid::OpticalDisc::DvdPlusRewritable:
                if (disc.isBlank())
                    description = QObject::tr("Blank DVD+RW");
                else
                    description = QObject::tr("DVD+RW");
                break;

            case Solid::OpticalDisc::DvdPlusRewritableDuallayer:
                if (disc.isBlank())
                    description = QObject::tr("Blank DVD+RW Dual-Layer");
                else
                    description = QObject::tr("DVD+RW Dual-Layer");
                break;

            case Solid::OpticalDisc::BluRayRom:
                description = QObject::tr("BD-ROM");
                break;

            case Solid::OpticalDisc::BluRayRecordable:
                if (disc.isBlank())
                    description = QObject::tr("Blank BD-R");
                else
                    description = QObject::tr("BD-R");
                break;

            case Solid::OpticalDisc::BluRayRewritable:
                if (disc.isBlank())
                    description = QObject::tr("Blank BD-RE");
                else
                    description = QObject::tr("BD-RE");
                break;

            case Solid::OpticalDisc::HdDvdRom:
                description = QObject::tr("HD DVD-ROM");
                break;

            case Solid::OpticalDisc::HdDvdRecordable:
                if (disc.isBlank())
                    description = QObject::tr("Blank HD DVD-R");
                else
                    description = QObject::tr("HD DVD-R");
                break;

            case Solid::OpticalDisc::HdDvdRewritable:
                if (disc.isBlank())
                    description = QObject::tr("Blank HD DVD-RW");
                else
                    description = QObject::tr("HD DVD-RW");
                break;
            }

        // Special case for pure audio disc
        if (disc.availableContent() == Solid::OpticalDisc::Audio)
            description = QObject::tr("Audio CD");

        return description;
    }

    const bool drive_is_removable = storageDrive.isRemovable();
    const bool drive_is_hotpluggable = storageDrive.isHotpluggable();

    QString size_str = formatByteSize(storageVolume.size());
    if (isEncryptedContainer())
    {
        if (!size_str.isEmpty())
            description = QObject::tr("%1 Encrypted Container", "%1 is the size").arg(size_str);
        else
            description = QObject::tr("Encrypted Container");
    }
    else if (drive_type == Solid::StorageDrive::HardDisk && !drive_is_removable)
    {
        if (!size_str.isEmpty())
        {
            if (drive_is_hotpluggable)
                description = QObject::tr("%1 External Hard Drive", "%1 is the size").arg(size_str);
            else
                description = QObject::tr("%1 Hard Drive", "%1 is the size").arg(size_str);
        }
        else
        {
            if (drive_is_hotpluggable)
                description = QObject::tr("External Hard Drive");
            else
                description = QObject::tr("Hard Drive");
        }
    }
    else
    {
        if (drive_is_removable)
            description = QObject::tr("%1 Removable Media", "%1 is the size").arg(size_str);
        else
            description = QObject::tr("%1 Media", "%1 is the size").arg(size_str);
    }

    return description;
}

QString Device::icon() const
{
    QString iconName = prop( "HintIconName" ).toString();

    if ( !iconName.isEmpty() )
    {
        return iconName;
    }
    else if (isDrive()) {
        const bool isRemovable = prop("Removable").toBool();
        const QString conn = prop("ConnectionBus").toString();

        if (isOpticalDrive())
            return "drive-optical";
        else if (isRemovable && !isOpticalDisc()) {
            if (conn == "usb")
                return "drive-removable-media-usb";
            else
                return "drive-removable-media";
        }
    }
    else if (isBlock()) {
        const QString drivePath = prop("Drive").value<QDBusObjectPath>().path();
        Device drive(drivePath);

        // handle media
        const QString media = drive.prop("Media").toString();

        if ( !media.isEmpty() )
        {
            if ( isOpticalDisc() )    // optical stuff
            {
                bool isWritable = drive.prop("OpticalBlank").toBool();

                const UDisks2::OpticalDisc disc(const_cast<Device*>(this));
                Solid::OpticalDisc::ContentTypes availContent = disc.availableContent();

                if (availContent & Solid::OpticalDisc::VideoDvd) // Video DVD
                    return "media-optical-dvd-video";
                else if ((availContent & Solid::OpticalDisc::VideoCd) || (availContent & Solid::OpticalDisc::SuperVideoCd)) // Video CD
                    return "media-optical-video";
                else if ((availContent & Solid::OpticalDisc::Data) && (availContent & Solid::OpticalDisc::Audio)) // Mixed CD
                    return "media-optical-mixed-cd";
                else if (availContent & Solid::OpticalDisc::Audio) // Audio CD
                    return "media-optical-audio";
                else if (availContent & Solid::OpticalDisc::Data) // Data CD
                    return "media-optical-data";
                else if ( isWritable )
                    return "media-optical-recordable";
                else
                {
                    if ( media.startsWith( "optical_dvd" ) || media.startsWith( "optical_hddvd" ) ) // DVD
                        return "media-optical-dvd";
                    else if ( media.startsWith( "optical_bd" ) ) // BluRay
                        return "media-optical-blu-ray";
                }

                // fallback for every other optical disc
                return "media-optical";
            }

            if ( media == "flash_ms" ) // Flash & Co.
                return "media-flash-memory-stick";
            else if ( media == "flash_sd" || media == "flash_sdhc" || media == "flash_sdxc" || media == "flash_mmc" )
                return "media-flash-sd-mmc";
            else if ( media == "flash_sm" )
                return "media-flash-smart-media";
            else if ( media == "thumb" )
                return "drive-removable-media-usb-pendrive";
            else if ( media.startsWith( "flash" ) )
                return "media-flash";
            else if ( media == "floppy" ) // the good ol' floppy
                return "media-floppy";
        }

        if (drive.prop("ConnectionBus").toString() == "sdio")  // hack for SD cards connected thru sdio bus
            return "media-flash-sd-mmc";

        return drive.icon();
    }

    return "drive-harddisk";    // general fallback
}

QString Device::product() const
{
    QString product = prop("Model").toString();

    if (!isDrive()) {
        QString label = prop("IdLabel").toString();
        if (!label.isEmpty()) {
            product = label;
        }
    }

    return product;
}

QString Device::vendor() const
{
    return prop("Vendor").toString();
}

QString Device::udi() const
{
    return m_udi;
}

QString Device::parentUdi() const
{
    QString parent;

    if (isEncryptedContainer())
        parent = prop("CryptoBackingDevice").value<QDBusObjectPath>().path();
    else if (propertyExists("Drive"))  // block
        parent = prop("Drive").value<QDBusObjectPath>().path();
    else if (propertyExists("Table"))  // partition
        parent = prop("Table").value<QDBusObjectPath>().path();
    else if (parent.isEmpty() || parent=="/") {
        parent = UD2_UDI_DISKS_PREFIX;
    }
    return parent;
}

void Device::checkCache(const QString &key) const
{
    if (m_cache.isEmpty()) // recreate the cache
        allProperties();

    if (m_cache.contains(key))
        return;

    QVariant reply = m_device->property(key.toUtf8());

    if (reply.isValid()) {
        m_cache.insert(key, reply);
    } else {
        //qDebug() << "got invalid reply for cache:" << key;
        //m_cache.insert(key, QVariant());
    }
}

QString Device::introspect() const
{
    QDBusMessage call = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, m_udi,
                                                       DBUS_INTERFACE_INTROSPECT, "Introspect");
    QDBusPendingReply<QString> reply = QDBusConnection::systemBus().asyncCall(call);
    reply.waitForFinished();

    if (reply.isValid())
        return reply.value();
    else {
        return QString();
    }
}

QVariant Device::prop(const QString &key) const
{
    checkCache(key);
    return m_cache.value(key);
}

bool Device::propertyExists(const QString &key) const
{
    checkCache(key);
    return m_cache.contains(key);
}

QVariantMap Device::allProperties() const
{
    Q_FOREACH (const QString & iface, m_interfaces) {
        if (iface.startsWith("org.freedesktop.DBus"))
            continue;
        QDBusMessage call = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, m_udi, DBUS_INTERFACE_PROPS, "GetAll");
        call << iface;
        QDBusPendingReply<QVariantMap> reply = QDBusConnection::systemBus().asyncCall(call);
        reply.waitForFinished();

        if (reply.isValid())
            m_cache.unite(reply.value());
        else
            qWarning() << "Error getting props:" << reply.error().name() << reply.error().message();
        //qDebug() << "After iface" << iface << ", cache now contains" << m_cache.size() << "items";
    }

    return m_cache;
}

bool Device::hasInterface(const QString &name) const
{
    return m_interfaces.contains(name);
}

QStringList Device::interfaces() const
{
    return m_interfaces;
}

void Device::initInterfaces()
{
    m_interfaces.clear();
    const QString xmlData = introspect();
    QDomDocument dom;
    dom.setContent(xmlData);
    QDomNodeList ifaceNodeList = dom.elementsByTagName("interface");
    for (int i = 0; i < ifaceNodeList.count(); i++) {
        QDomElement ifaceElem = ifaceNodeList.item(i).toElement();
        if (!ifaceElem.isNull())
            m_interfaces.append(ifaceElem.attribute("name"));
    }
    //qDebug() << "Device" << m_udi << "has interfaces:" << m_interfaces;
}

void Device::slotPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    //Q_UNUSED(ifaceName);

    qDebug() << m_udi << "'s interface" << ifaceName << "changed props:";

    QMap<QString, int> changeMap;

    Q_FOREACH(const QString & key, invalidatedProps) {
        m_cache.remove(key);
        changeMap.insert(key, Solid::GenericInterface::PropertyRemoved);
        qDebug() << "\t invalidated:" << key;
    }

    QMapIterator<QString, QVariant> i(changedProps);
    while (i.hasNext()) {
        i.next();
        const QString key = i.key();
        m_cache.insert(key, i.value());  // replace the value
        changeMap.insert(key, Solid::GenericInterface::PropertyModified);
        qDebug() << "\t modified:" << key << ":" << m_cache.value(key);
    }

    Q_EMIT propertyChanged(changeMap);
    Q_EMIT changed();
}

void Device::slotInterfacesAdded(const QDBusObjectPath &object_path, const QVariantMapMap &interfaces_and_properties)
{
    if (object_path.path() == m_udi) {
        m_interfaces.append(interfaces_and_properties.keys());
    }
}

void Device::slotInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    if (object_path.path() == m_udi) {
        Q_FOREACH(const QString & iface, interfaces) {
            m_interfaces.removeAll(iface);
        }
    }
}


QString Device::errorToString(const QString & error) const
{
    if (error == UD2_ERROR_UNAUTHORIZED || error == UD2_ERROR_NOT_AUTHORIZED)
        return QObject::tr("You are not authorized to perform this operation");
    else if (error == UD2_ERROR_BUSY)
        return QObject::tr("The device is currently busy");
    else if (error == UD2_ERROR_FAILED)
        return QObject::tr("The requested operation has failed");
    else if (error == UD2_ERROR_CANCELED)
        return QObject::tr("The requested operation has been canceled");
    else if (error == UD2_ERROR_INVALID_OPTION)
        return QObject::tr("An invalid or malformed option has been given");
    else if (error == UD2_ERROR_MISSING_DRIVER)
        return QObject::tr("The kernel driver for this filesystem type is not available");
    else if (error == UD2_ERROR_ALREADY_MOUNTED)
        return QObject::tr("The device is already mounted");
    else if (error == UD2_ERROR_NOT_MOUNTED)
        return QObject::tr("The device is not mounted");
    else if (error == UD2_ERROR_MOUNTED_BY_OTHER_USER)
        return QObject::tr("The device is mounted by another user");
    else if (error == UD2_ERROR_ALREADY_UNMOUNTING)
        return QObject::tr("The device is already unmounting");
    else if (error == UD2_ERROR_TIMED_OUT)
        return QObject::tr("The operation timed out");
    else if (error == UD2_ERROR_WOULD_WAKEUP)
        return QObject::tr("The operation would wake up a disk that is in a deep-sleep state");
    else if (error == UD2_ERROR_ALREADY_CANCELLED)
        return QObject::tr("The operation has already been canceled");
    else
        return QObject::tr("An unspecified error has occurred");
}

Solid::ErrorType Device::errorToSolidError(const QString & error) const
{
    if (error == UD2_ERROR_BUSY)
        return Solid::DeviceBusy;
    else if (error == UD2_ERROR_FAILED)
        return Solid::OperationFailed;
    else if (error == UD2_ERROR_CANCELED)
        return Solid::UserCanceled;
    else if (error == UD2_ERROR_INVALID_OPTION)
        return Solid::InvalidOption;
    else if (error == UD2_ERROR_MISSING_DRIVER)
        return Solid::MissingDriver;
    else
        return Solid::UnauthorizedOperation;
}

bool Device::isBlock() const
{
    return hasInterface(UD2_DBUS_INTERFACE_BLOCK);
}

bool Device::isPartition() const
{
    return hasInterface(UD2_DBUS_INTERFACE_PARTITION);
}

bool Device::isPartitionTable() const
{
    return hasInterface(UD2_DBUS_INTERFACE_PARTITIONTABLE);
}

bool Device::isStorageVolume() const
{
    return isPartition() || isPartitionTable() || isStorageAccess() || isOpticalDisc();
}

bool Device::isStorageAccess() const
{
    return hasInterface(UD2_DBUS_INTERFACE_FILESYSTEM) || isEncryptedContainer();
}

bool Device::isDrive() const
{
    return hasInterface(UD2_DBUS_INTERFACE_DRIVE);
}

bool Device::isOpticalDrive() const
{
    return isDrive() && !prop("MediaCompatibility").toStringList().filter("optical_").isEmpty();
}

bool Device::isOpticalDisc() const
{
    const QString drivePath = prop("Drive").value<QDBusObjectPath>().path();
    if (drivePath.isEmpty() || drivePath == "/")
        return false;

    Device drive(drivePath);
    return drive.prop("Optical").toBool();
}

bool Device::mightBeOpticalDisc() const
{
    const QString drivePath = prop("Drive").value<QDBusObjectPath>().path();
    if (drivePath.isEmpty() || drivePath == "/")
        return false;

    Device drive(drivePath);
    return drive.isOpticalDrive();
}

bool Device::isMounted() const
{
    return propertyExists("MountPoints") && !qdbus_cast<QByteArrayList>(prop("MountPoints")).isEmpty();
}

bool Device::isEncryptedContainer() const
{
    return hasInterface(UD2_DBUS_INTERFACE_ENCRYPTED);
}

bool Device::isEncryptedCleartext() const
{
    const QString holderDevice = prop("CryptoBackingDevice").toString();
    if (holderDevice.isEmpty() || holderDevice == "/")
        return false;
    else
        return true;
}

bool Device::isSwap() const
{
    return hasInterface(UD2_DBUS_INTERFACE_SWAP);
}
