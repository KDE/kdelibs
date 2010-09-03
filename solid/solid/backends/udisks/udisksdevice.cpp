/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>
              2010  Lukas Tinkl <ltinkl@redhat.com>

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
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "udisks.h"
#include "udisksdevice.h"
#include "udisksdeviceinterface.h"
#include "udisksstoragevolume.h"
#include "udisksopticaldisc.h"
#include "udisksopticaldrive.h"
#include "udisksstorageaccess.h"
#include "udisksgenericinterface.h"

#include <solid/genericinterface.h>
#include <solid/deviceinterface.h>
#include <solid/device.h>

#include <QtCore/QStringList>
#include <QtDBus/QDBusReply>
#include <QtCore/QDebug>

using namespace Solid::Backends::UDisks;

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

UDisksDevice::UDisksDevice(const QString &udi)
    : m_udi(udi)
{
    QString realUdi = m_udi;
    if (realUdi.endsWith(":media")) {
        realUdi.chop(6);
    }
    m_device = new QDBusInterface(UD_DBUS_SERVICE, realUdi,
                                  UD_DBUS_INTERFACE_DISKS_DEVICE,
                                  QDBusConnection::systemBus());

    connect(m_device, SIGNAL(Changed()), this, SLOT(slotChanged()));
}

UDisksDevice::~UDisksDevice()
{

}

QObject* UDisksDevice::createDeviceInterface(const Solid::DeviceInterface::Type& type)
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
        iface = new UDisksStorageAccess(this);
        break;
    case Solid::DeviceInterface::StorageDrive:
        iface = new UDisksStorageDrive(this);
        break;
    case Solid::DeviceInterface::OpticalDrive:
        iface = new UDisksOpticalDrive(this);
        break;
    case Solid::DeviceInterface::StorageVolume:
        iface = new UDisksStorageVolume(this);
        break;
    case Solid::DeviceInterface::OpticalDisc:
        iface = new OpticalDisc(this);
        break;
    default:
        break;
    }
    return iface;
}

bool UDisksDevice::queryDeviceInterface(const Solid::DeviceInterface::Type& type) const
{
    switch (type) {
        case Solid::DeviceInterface::GenericInterface:
            return true;
        case Solid::DeviceInterface::Block:
            return property("DeviceBlockSize").toULongLong() > 0;
        case Solid::DeviceInterface::StorageVolume:
            return property("DeviceIsPartition").toBool()
                    || (property("DeviceIsOpticalDisc").toBool() && m_udi.endsWith(":media"));
        case Solid::DeviceInterface::StorageAccess:
            return (property("DeviceIsPartition").toBool() && property("PartitionNumber").toInt() > 0)
                    || (property("DeviceIsOpticalDisc").toBool() && m_udi.endsWith(":media"));
        case Solid::DeviceInterface::StorageDrive:
            return !m_udi.endsWith(":media") && property("DeviceIsDrive").toBool();
        case Solid::DeviceInterface::OpticalDrive:
            return !m_udi.endsWith(":media")
                    && property( "DeviceIsDrive" ).toBool()
                    && !property( "DriveMediaCompatibility" ).toStringList().filter( "optical_" ).isEmpty();
        case Solid::DeviceInterface::OpticalDisc:
            return m_udi.endsWith(":media") && property("DeviceIsOpticalDisc").toBool();
        default:
            return false;
    }
}

QStringList UDisksDevice::emblems() const
{
    QStringList res;

    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess))
    {

        bool isEncrypted = false;
        if (queryDeviceInterface(Solid::DeviceInterface::StorageVolume))
        {
            const UDisks::UDisksStorageVolume volIface(const_cast<UDisksDevice *>(this));
            isEncrypted = (volIface.usage() == Solid::StorageVolume::Encrypted);
        }

        const UDisks::UDisksStorageAccess accessIface(const_cast<UDisksDevice *>(this));
        if (accessIface.isAccessible())
        {
            if (isEncrypted)
                res << "emblem-encrypted-unlocked";
            else
                res << "emblem-mounted";
        }
        else
        {
            if (isEncrypted)
                res << "emblem-encrypted-locked";
            else
                res << "emblem-unmounted";
        }
    }

    return res;
}

QString UDisksDevice::description() const
{
    if (queryDeviceInterface(Solid::DeviceInterface::StorageAccess))
        return storageDescription();
    else if (queryDeviceInterface(Solid::DeviceInterface::StorageVolume))
        return volumeDescription();
#if 0 // FIXME after implementing NetworkInterface
    else if (queryDeviceInterface(Solid::DeviceInterface::NetworkInterface))
    {
        const Solid::DeviceInterface::NetworkInterface netIface(const_cast<UDisksDevice *>(this));
        if (netIface.isWireless())
            return QObject::tr("WLAN Interface");
        else
            return QObject::tr("Networking Interface");
    }
#endif
    else
        return product();
}

QString UDisksDevice::storageDescription() const
{
    QString description;
    const UDisks::UDisksStorageDrive storageDrive(const_cast<UDisksDevice*>(this));
    Solid::StorageDrive::DriveType drive_type = storageDrive.driveType();
    bool drive_is_hotpluggable = storageDrive.isHotpluggable();
    const UDisks::UDisksStorageVolume storageVolume(const_cast<UDisksDevice*>(this));

    if (drive_type == Solid::StorageDrive::CdromDrive)
    {
        const UDisks::UDisksOpticalDrive opticalDrive(const_cast<UDisksDevice*>(this));
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
                second = QObject::tr("/DVD±R DL", "Second item of %1%2 Drive sentence");
            else
                second = QObject::tr("/DVD±R", "Second item of %1%2 Drive sentence");
        }
        if ((mediumTypes & Solid::OpticalDrive::Dvdrw) && (mediumTypes & Solid::OpticalDrive::Dvdplusrw))
        {
            if((mediumTypes & Solid::OpticalDrive::Dvdplusdl) || (mediumTypes & Solid::OpticalDrive::Dvdplusdlrw))
                second = QObject::tr("/DVD±RW DL", "Second item of %1%2 Drive sentence");
            else
                second = QObject::tr("/DVD±RW", "Second item of %1%2 Drive sentence");
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
            description = QObject::tr("External %1%2 Drive").arg(first).arg(second);
        else
            description = QObject::tr("%1%2 Drive").arg(first).arg(second);

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
        QString size_str = formatByteSize(storageVolume.size());
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
            vendormodel_str = QObject::tr("%1 %2", "%1 is the vendor, %2 is the model of the device").arg(vendor_str).arg(model);
    }

    if (vendormodel_str.isEmpty())
        description = QObject::tr("Drive");
    else
        description = vendormodel_str;

    return description;
}

QString UDisksDevice::volumeDescription() const
{
    QString description;
    const UDisks::UDisksStorageVolume storageVolume(const_cast<UDisksDevice*>(this));
    QString volume_label = storageVolume.label();

    if (!volume_label.isEmpty())
        return volume_label;

    const UDisks::UDisksStorageDrive storageDrive(const_cast<UDisksDevice*>(this));
    Solid::StorageDrive::DriveType drive_type = storageDrive.driveType();

    // Handle media in optical drives
    if (drive_type == Solid::StorageDrive::CdromDrive)
    {
        const UDisks::OpticalDisc disc(const_cast<UDisksDevice*>(this));
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

    bool drive_is_removable = storageDrive.isRemovable();
    bool drive_is_hotpluggable = storageDrive.isHotpluggable();
    bool drive_is_encrypted_container = (storageVolume.usage() == Solid::StorageVolume::Encrypted);

    QString size_str = formatByteSize(storageVolume.size());
    if (drive_is_encrypted_container)
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

QString UDisksDevice::icon() const
{
    QString iconName = property( "DevicePresentationIconName" ).toString();

    if ( !iconName.isEmpty() )
    {
        return iconName;
    }
    else
    {
        bool isPartition = property( "DeviceIsPartition" ).toBool();
        if ( isPartition )      // this is a slave device, we need to return its parent's icon
        {
            UDisksDevice * parent;
            if ( !parentUdi().isEmpty() )
                parent = new UDisksDevice( parentUdi() );

            if ( parent )
            {
                iconName = parent->icon();
                delete parent;
            }

            if ( !iconName.isEmpty() )
                return iconName;
        }

        // handle media
        const QString media = property( "DriveMedia" ).toString();
        bool isOptical = property( "DeviceIsOpticalDisc" ).toBool();

        if ( !media.isEmpty() )
        {
            if ( isOptical )    // optical stuff
            {
                bool isWritable = property( "OpticalDiscIsBlank" ).toBool() || property("OpticalDiscIsAppendable").toBool();

                const UDisks::OpticalDisc disc(const_cast<UDisksDevice*>(this));
                Solid::OpticalDisc::ContentTypes availContent = disc.availableContent();

                if (availContent & Solid::OpticalDisc::VideoDvd) // Video DVD
                    return "media-optical-dvd-video";
                else if ((availContent & Solid::OpticalDisc::VideoCd) || (availContent & Solid::OpticalDisc::SuperVideoCd)) // Video CD
                    return "media-optical";
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
            else if ( media == "flash_sd" || media == "flash_sdhc" || media == "flash_mmc" )
                return "media-flash-sd-mmc";
            else if ( media == "flash_sm" )
                return "media-flash-smart-media";
            else if ( media.startsWith( "flash" ) )
                return "media-flash";
            else if ( media == "floppy" ) // the good ol' floppy
                return "media-floppy";

        }

        // handle drives
        bool isDrive = property( "DeviceIsDrive" ).toBool();
        bool isRemovable = property( "DeviceIsRemovable" ).toBool();
        const QString conn = property( "DriveConnectionInterface" ).toString();

        if ( isDrive && isOptical )
            return "drive-optical";
        else if ( isRemovable && !isOptical )
        {
            if ( conn == "usb" )
                return "drive-removable-media-usb";
            else
                return "drive-removable-media";
        }
    }

    return "drive-harddisk";    // general fallback
}

QString UDisksDevice::product() const
{
    return property("DriveModel").toString();
}

QString UDisksDevice::vendor() const
{
    return property("DriveVendor").toString();
}

QString UDisksDevice::udi() const
{
    return m_udi;
}

QString UDisksDevice::parentUdi() const
{
    if (m_udi.endsWith(QLatin1String(":media")))
    {
        QString result = m_udi;
        return result.remove(":media");
    }
    else
        return property("PartitionSlave").value<QDBusObjectPath>().path();
}

void UDisksDevice::checkCache(const QString &key) const
{
    if (m_cache.isEmpty()) // recreate the cache
        allProperties();

    if (m_cache.contains(key) && !m_invalidKeys.contains(key))
        return;
    
    QVariant reply = m_device->property(key.toUtf8());

    if (reply.isValid()) {
        m_cache[key] = reply;
    } else {
        m_cache[key] = QVariant();
    }

    m_invalidKeys.remove(key);
}

QVariant UDisksDevice::property(const QString &key) const
{
    checkCache(key);
    return m_cache.value(key);
}

bool UDisksDevice::propertyExists(const QString &key) const
{
    checkCache(key);
    return m_cache.contains(key);
}

QMap<QString, QVariant> UDisksDevice::allProperties() const
{
    QDBusMessage message = QDBusMessage::createMethodCall(m_device->service(), m_device->path(), QLatin1String("org.freedesktop.DBus.Properties"), QLatin1String("GetAll"));
    QList<QVariant> arguments;
    arguments << m_device->interface();
    message.setArguments(arguments);

    QDBusMessage reply = m_device->connection().call(message);

    if (reply.type() != QDBusMessage::ReplyMessage)
    {
        qWarning() << Q_FUNC_INFO << "error:" << reply;
        return QVariantMap();
    }

    m_cache = QDBusReply<QVariantMap>(reply).value();

    return m_cache;
}

void UDisksDevice::slotChanged()
{
    // given we cannot know which property/ies changed, clear the cache
    m_cache.clear();
    emit changed();
}
