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

#include "wmidevice.h"

#include <solid/genericinterface.h>

#include "wmiquery.h"
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

#include <QtCore/QDebug>

using namespace Solid::Backends::Wmi;

class Solid::Backends::Wmi::WmiDevicePrivate
{
public:
    WmiDevicePrivate(const QString &_udi)
        : parent(0)
        , m_udi(_udi)
        , m_wmiTable()
        , m_wmiProperty()
        , m_wmiValue()
    {    
    }

    ~WmiDevicePrivate()
    {
    }
    
    void discoverType()
    {
        if (!convertUDItoWMI(m_udi,m_wmiTable,m_wmiProperty,m_wmiValue))
            return;
        
        // todo: add nicer implementation to detect device type
        if (m_wmiTable == getWMITable(Solid::DeviceInterface::OpticalDrive))
            interfaceList << "Block" << "StorageDrive" << "OpticalDrive" << "StorageVolume";
    }
   
    const QString udi() const { return m_udi; }

    WmiQuery::ItemList sendQuery() 
    {
        QString query("SELECT * FROM " + m_wmiTable + " WHERE " + m_wmiProperty + "='" + m_wmiValue + '\'');
		WmiQuery::ItemList list = WmiQuery::instance().sendQuery(query);
        return list;
    }
    
    static bool convertUDItoWMI(const QString &udi, QString &wmiTable, QString &wmiProperty, QString &wmiValue)
    {
        QString _udi = udi;
        QStringList x = _udi.remove("/org/kde/solid/wmi/").split('/');
        if (x.size() != 3) {
            qDebug() << "invalid udi detected" << _udi;
            return false;
        }
        Solid::DeviceInterface::Type type = DeviceInterface::fromString(x[0]);
        wmiTable = getWMITable(type);
        wmiProperty = x[1];
        wmiValue = x[2];
        return true;
    }

    static bool exists(const QString &udi)
    {
        QString wmiTable;
        QString wmiProperty;
        QString wmiValue;
        
        if (!convertUDItoWMI(udi, wmiTable, wmiProperty, wmiValue))
            return false;

        QString query("SELECT * FROM " + wmiTable + " WHERE " + wmiProperty + "='" + wmiValue + '\'');
		WmiQuery::ItemList list = WmiQuery::instance().sendQuery(query);
        return list.size() > 0;
    }

    static QString generateUDI(const QString &key, const QString &property, const QString &value)
    {
        return QString("/org/kde/solid/wmi/%1/%2/%3").arg(key).arg(property).arg(value);
    }
    
    static QStringList getInterfaces(const Solid::DeviceInterface::Type &type)
    {
        QStringList interfaceList;
            
        switch (type)
        {
        case Solid::DeviceInterface::GenericInterface:
            break;
        case Solid::DeviceInterface::Processor:
            break;
        case Solid::DeviceInterface::Block:
            break;
        case Solid::DeviceInterface::StorageAccess:
            break;
        case Solid::DeviceInterface::StorageDrive:
            interfaceList << "Block" << "StorageDrive";
            break;
        case Solid::DeviceInterface::OpticalDrive:
            interfaceList << "Block" << "StorageDrive" << "OpticalDrive";
            break;
        case Solid::DeviceInterface::StorageVolume:
            interfaceList << "Block" << "StorageVolume";
            break;
        case Solid::DeviceInterface::OpticalDisc:
            interfaceList << "Block" << "StorageDrive" << "OpticalDrive";
            break;
        case Solid::DeviceInterface::Camera:
            break;
        case Solid::DeviceInterface::PortableMediaPlayer:
            break;
        case Solid::DeviceInterface::NetworkInterface:
            break;
        case Solid::DeviceInterface::AcAdapter:
            break;
        case Solid::DeviceInterface::Battery:
            break;
        case Solid::DeviceInterface::Button:
            break;
        case Solid::DeviceInterface::AudioInterface:
            break;
        case Solid::DeviceInterface::DvbInterface:
            break;        case Solid::DeviceInterface::Video:
            break;
        case Solid::DeviceInterface::Unknown:
        case Solid::DeviceInterface::Last:
        default:
            break;
        }
        if (interfaceList.size() == 0)
            qWarning() << "no interface found for type" << type;
        return interfaceList;
    }
    
    static QString getUDIKey(const Solid::DeviceInterface::Type &type)
    {
        QStringList list = DeviceInterface::toStringList(type);
        QString value = list.size() > 0 ? list[0] : QString();
        qDebug() << value;
        return value;
    }

    static QString getWMITable(const Solid::DeviceInterface::Type &type)
    {
        QString wmiTable;
        switch (type)
        {
        case Solid::DeviceInterface::GenericInterface:
            break;
        case Solid::DeviceInterface::Processor:
            wmiTable = "Win32_Processor";
            break;
        case Solid::DeviceInterface::Block:
            break;
        case Solid::DeviceInterface::StorageAccess:
            wmiTable = "Win32_LogicalDisk";
            break;
        case Solid::DeviceInterface::StorageDrive:
            break;
        case Solid::DeviceInterface::OpticalDrive:
            wmiTable = "Win32_LogicalDisk";
            break;
        case Solid::DeviceInterface::StorageVolume:
            wmiTable = "Win32_LogicalDisk";
            break;
        case Solid::DeviceInterface::OpticalDisc:
            wmiTable = "Win32_CDROMDrive";
            break;
        case Solid::DeviceInterface::Camera:
            break;
        case Solid::DeviceInterface::PortableMediaPlayer:
            break;
        case Solid::DeviceInterface::NetworkInterface:
            break;
        case Solid::DeviceInterface::AcAdapter:
            break;
        case Solid::DeviceInterface::Battery:
            wmiTable = "Win32_Battery";
            break;
        case Solid::DeviceInterface::Button:
            break;
        case Solid::DeviceInterface::AudioInterface:
            break;
        case Solid::DeviceInterface::DvbInterface:
            break;
        case Solid::DeviceInterface::Video:
            break;
        case Solid::DeviceInterface::Unknown:
        case Solid::DeviceInterface::Last:
        default:
            wmiTable = "unknown";
            break;
        }
        return wmiTable;
    }    

    static QString getPropertyNameForUDI(const Solid::DeviceInterface::Type &type)
    {
        QString propertyName;
        if (type == Solid::DeviceInterface::OpticalDrive)
            propertyName = "Drive";
        else if (type == Solid::DeviceInterface::Battery)
            propertyName = "Name";
        else
            propertyName = "DeviceID";    
            
        return propertyName;
    }
    
    static QStringList generateUDIList(const Solid::DeviceInterface::Type &type)
    {
        QStringList result;
        
		WmiQuery::ItemList list = WmiQuery::instance().sendQuery( "select * from " + getWMITable(type) );
        foreach(WmiQuery::Item *item, list) {
            QString propertyName = getPropertyNameForUDI(type);
            QString property = item->getProperty(propertyName);
            
            result << generateUDI(getUDIKey(type),propertyName.toLower(),property.toLower());
        }
        return result;
    }
    
    WmiDevice *parent;
    static int m_instanceCount;
    QString m_udi;
    QString m_wmiTable;
    QString m_wmiProperty;
    QString m_wmiValue;
    QStringList interfaceList;
};

Q_DECLARE_METATYPE(ChangeDescription)
Q_DECLARE_METATYPE(QList<ChangeDescription>)
WmiDevice::WmiDevice(const QString &udi)
    : Device(), d(new WmiDevicePrivate(udi))
{
    d->discoverType();
    foreach (const QString &_interface, d->interfaceList)
    {
        Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(_interface);
        createDeviceInterface(type);
    }
}

WmiDevice::~WmiDevice()
{
    //delete d->parent;
    delete d;
}

QStringList WmiDevice::generateUDIList(const Solid::DeviceInterface::Type &type)
{
    return WmiDevicePrivate::generateUDIList(type);
}

bool WmiDevice::exists(const QString &udi)
{
    return WmiDevicePrivate::exists(udi);
}

bool WmiDevice::isValid() const
{
    // does not work 
    //return sendQuery( "SELECT * FROM Win32_SystemDevices WHERE PartComponent='\\\\\\\\BEAST\root\cimv2:Win32_Processor.DeviceID=\"CPU0\"'" ).count() == 1;
    return true;
}

QString WmiDevice::udi() const
{
    return d->udi();
}

QString WmiDevice::parentUdi() const
{
    return QString();
}

QString WmiDevice::vendor() const
{
    return property("Manufacturer").toString();
}

QString WmiDevice::product() const
{
    return property("Name").toString();
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

QStringList WmiDevice::emblems() const
{
    return QStringList(); // TODO
}

QString WmiDevice::description() const
{
    return product(); // TODO
}

QVariant WmiDevice::property(const QString &key) const
{    
    WmiQuery::ItemList list = d->sendQuery();
    if (list.size() == 0)
        return QString();

    QString result = list[0]->getProperty( key );
    return result;
}

QMap<QString, QVariant> WmiDevice::allProperties() const
{
    // QDBusReply<QVariantMap> reply = d->device.call("GetAllProperties");

    // if (!reply.isValid())
    // {
        // qWarning() << Q_FUNC_INFO << " error: " << reply.error().name()
                   // << ", " << reply.error().message() << endl;
        // return QVariantMap();
    // }

    //return reply;
    return QMap<QString,QVariant>();
}

bool WmiDevice::propertyExists(const QString &key) const
{
    WmiQuery::ItemList list = d->sendQuery();
    if (list.size() == 0)
        return false;
    return list[0]->getProperty( key ).isEmpty() ? false: true;
}

bool WmiDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    // Special cases not matching with WMI capabilities
    if (type==Solid::DeviceInterface::GenericInterface) {
        return true;
    } else if (type==Solid::DeviceInterface::StorageAccess) {
#if 1
        qDebug() << " has to be implemented"; 
        return true;
#else
        return property("info.interfaces").toStringList().contains("org.freedesktop.Wmi.Device.Volume")
            || property("info.interfaces").toStringList().contains("org.freedesktop.Wmi.Device.Volume.Crypto");
#endif
    }
    else if (type==Solid::DeviceInterface::Video) {
        if (!property("video4linux.device").toString().contains("video" ) )
          return false;
    }

    return d->interfaceList.contains(Solid::DeviceInterface::typeToString(type));
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
    }

    emit propertyChanged(result);
}

void WmiDevice::slotCondition(const QString &condition, const QString &reason)
{
    emit conditionRaised(condition, reason);
}

#include "backends/wmi/wmidevice.moc"
