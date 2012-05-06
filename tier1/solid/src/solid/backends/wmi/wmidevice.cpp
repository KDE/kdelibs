/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
    Copyright 2005-2007 Kevin Ottens <ervin@kde.org>

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

#include "wmidevice.h"

#include <solid/genericinterface.h>

#include "wmiquery.h"
#include "wmimanager.h"
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

QString& WmiDevice::driveLetterToUid(const QString &letter){
    static QMap<QString,QString> uids;
    static QRegExp uidreg("([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})");
    if(uids.isEmpty()){
        QString query("SELECT * FROM Win32_Volume");
        WmiQuery::ItemList list = WmiQuery::instance().sendQuery(query);
        Q_FOREACH(const WmiQuery::Item & item,list){
            uidreg.indexIn(item.getProperty("DeviceID").toString());
            QString key = item.getProperty("DriveLetter").toString().toLower();
            QString value = uidreg.capturedTexts()[0];
            uids[key] = value;
        }
    }
    return uids[letter.toLower()];
}

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
        if (!convertUDItoWMI(m_udi,m_wmiTable,m_wmiProperty,m_wmiValue,m_type))
            return;
        interfaceList<<getInterfaces(m_type);

    }

    const QString udi() const { return m_udi; }

    WmiQuery::Item sendQuery()
    {
        if(m_list.isEmpty()){
            QString query("SELECT * FROM " + m_wmiTable + " WHERE " + m_wmiProperty + "='" + m_wmiValue + "'");
            WmiQuery::ItemList items = WmiQuery::instance().sendQuery(query);
            Q_ASSERT(items.length() != 1);
            if(items.length() != 1)
                qDebug()<<"WmiDevicePrivate::sendQuery() failed";
            m_list = items;
        }
        return m_list[0];
    }

    static bool convertUDItoWMI(const QString &udi, QString &wmiTable, QString &wmiProperty, QString &wmiValue,Solid::DeviceInterface::Type &type)
    {
        QString _udi = udi;
        QStringList x = _udi.remove("/org/kde/solid/wmi/").split('/');
        if (x.size() != 2 || x[1].isEmpty()) {
            qDebug() << "invalid udi detected" << udi;
            return false;
        }
        type = DeviceInterface::fromString(x[0]);
        wmiTable = getWMITable(type);        
        wmiProperty = getPropertyNameForUDI(type);
        wmiValue = x[1];        
        if(type == Solid::DeviceInterface::StorageVolume )
            wmiValue = "\\\\\\\\?\\\\Volume{"+wmiValue+"}\\\\";
//        qDebug()<<"wmi"<<  type <<wmiTable <<wmiProperty <<wmiValue;
        return true;
    }

    static bool exists(const QString &udi)
    {
        QString wmiTable;
        QString wmiProperty;
        QString wmiValue;
        Solid::DeviceInterface::Type solidDevice;

        if (!convertUDItoWMI(udi, wmiTable, wmiProperty, wmiValue,solidDevice))
            return false;


        QString query("SELECT * FROM " + wmiTable + " WHERE " + wmiProperty + "='" + wmiValue + "'");
		WmiQuery::ItemList list = WmiQuery::instance().sendQuery(query);
        if(list.size() > 0)
            return true;
        qWarning()<<"Device UDI:"<<udi<<"doesnt exist";
//        qDebug()<<query;
        return false;
    }

    static QString generateUDI(const QString &key, const QString &property)
    {
        return QString("/org/kde/solid/wmi/%1/%2").arg(key).arg(property);
    }

    static QList<Solid::DeviceInterface::Type> getInterfaces(const Solid::DeviceInterface::Type &type)
    {
        QList<Solid::DeviceInterface::Type> interfaceList;
        interfaceList<<type;

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
            break;
        case Solid::DeviceInterface::OpticalDrive:
            interfaceList <<Solid::DeviceInterface::StorageDrive;
            break;
        case Solid::DeviceInterface::StorageVolume:
            interfaceList <<Solid::DeviceInterface::StorageAccess;
            break;
        case Solid::DeviceInterface::OpticalDisc:
            interfaceList <<Solid::DeviceInterface::StorageDrive<<Solid::DeviceInterface::OpticalDrive;
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
            break;
        case Solid::DeviceInterface::Video:
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
            wmiTable = "Win32_DiskDrive";
            break;
        case Solid::DeviceInterface::OpticalDrive:
            wmiTable = "Win32_LogicalDisk";
            break;
        case Solid::DeviceInterface::StorageVolume:
            wmiTable = "Win32_Volume";
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
        switch(type){
        case Solid::DeviceInterface::OpticalDrive:
            propertyName = "Drive";
            break;
        case Solid::DeviceInterface::Battery:
            propertyName = "Name";
            break;
        case Solid::DeviceInterface::StorageAccess:
             propertyName = "DeviceID";
             break;
        case Solid::DeviceInterface::StorageVolume:
            propertyName = "DeviceID";
            break;
        case Solid::DeviceInterface::StorageDrive:
            propertyName = "Index";
            break;
        default:
            propertyName = "DeviceID";
        }

        return propertyName;
    }

    static QString getIgnorePatternForUDI(const Solid::DeviceInterface::Type &type)
    {
        QString ignorePattern;
        switch(type){
        case Solid::DeviceInterface::OpticalDrive:
            break;
        case Solid::DeviceInterface::Battery:
            break;
            //ignore cd for now
        case Solid::DeviceInterface::StorageAccess:
        case Solid::DeviceInterface::StorageVolume:
            ignorePattern = " WHERE DriveType != 5";
            break;
        case Solid::DeviceInterface::StorageDrive:
            break;
        default:
            ignorePattern = "";
        }

        return ignorePattern;
    }

    static QString getPropertyNameForVendor(const Solid::DeviceInterface::Type &type)
    {
        QString propertyName;
        switch(type){
        case Solid::DeviceInterface::Processor:
            propertyName = "Manufacturer";
            break;
        case Solid::DeviceInterface::OpticalDrive:
            propertyName = "Drive";//TODO:
            break;
        case Solid::DeviceInterface::Battery:
            propertyName = "Name";//TODO:
            break;
        case Solid::DeviceInterface::StorageAccess:
            propertyName = "DeviceID";//TODO:
            break;
        case Solid::DeviceInterface::StorageVolume:
            propertyName = "DriveLetter";//TODO:
            break;
        case Solid::DeviceInterface::StorageDrive:
            propertyName = "Model";
            break;
        default:
            propertyName = "DeviceID";//TODO:
        }
        return propertyName;
    }

    static QString getPropertyNameForProduct(const Solid::DeviceInterface::Type &type)
    {
        QString propertyName;
        switch(type){
        case Solid::DeviceInterface::Processor:
            propertyName = "Name";
            break;
        case Solid::DeviceInterface::OpticalDrive:
            propertyName = "Drive";//TODO:
            break;
        case Solid::DeviceInterface::Battery:
            propertyName = "Name";//TODO:
            break;
        case Solid::DeviceInterface::StorageAccess:
             propertyName = "DeviceID";//TODO:
             break;
        case Solid::DeviceInterface::StorageVolume:
            propertyName = "Label";
            break;
        case Solid::DeviceInterface::StorageDrive:
            propertyName = "Caption";
            break;
        default:
            propertyName = "DeviceID";//TODO:
        }
        return propertyName;
    }



    static QStringList generateUDIList(const Solid::DeviceInterface::Type &type)
    {
        QStringList result;

        WmiQuery::ItemList list = WmiQuery::instance().sendQuery( "select * from " + getWMITable(type) + getIgnorePatternForUDI(type) );
        Q_FOREACH(const WmiQuery::Item& item, list) {
            QString propertyName = getPropertyNameForUDI(type);
            QString property = item.getProperty(propertyName).toString();
            if(type == Solid::DeviceInterface::StorageVolume ){
                static QRegExp uid("([0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12})");
                uid.indexIn(property);
                property =  uid.capturedTexts()[0];
            }else if(type == Solid::DeviceInterface::StorageAccess ){
                property = WmiDevice::driveLetterToUid(property);
                if(property.isEmpty())//as far as I know this can only happen with subst drives
                    continue;
            }
            result << generateUDI(getUDIKey(type),property.toLower());
        }
        return result;
    }

    WmiDevice *parent;
    static int m_instanceCount;
    QString m_parent_uid;
    QString m_udi;
    QString m_wmiTable;
    QString m_wmiProperty;
    QString m_wmiValue;
    Solid::DeviceInterface::Type m_type;
    WmiQuery::ItemList m_list;
    QList<Solid::DeviceInterface::Type> interfaceList;
};

Q_DECLARE_METATYPE(ChangeDescription)
Q_DECLARE_METATYPE(QList<ChangeDescription>)
WmiDevice::WmiDevice(const QString &udi)
    : Device(), d(new WmiDevicePrivate(udi))
{
    d->discoverType();
    Q_FOREACH (Solid::DeviceInterface::Type type, d->interfaceList)
    {
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
    if(!d->m_parent_uid.isEmpty())
        return d->m_parent_uid;
    QString result;
    const QString value = udi().split("/").last();

    if(d->m_type == Solid::DeviceInterface::StorageVolume){
        QString id = property("DriveLetter").toString();
        QString query = "ASSOCIATORS OF {Win32_LogicalDisk.DeviceID='"+id+"'} WHERE AssocClass = Win32_LogicalDiskToPartition";
        WmiQuery::ItemList list = WmiQuery::instance().sendQuery(query);
        if(list.length() == 1){
            id = list[0].getProperty("Name").toString();
            query = "ASSOCIATORS OF {Win32_DiskPartition.DeviceID='"+id+"'} WHERE AssocClass = Win32_DiskDriveToDiskPartition";
            list = WmiQuery::instance().sendQuery(query);
            result = "/org/kde/solid/wmi/storage/"+list[0].getProperty("Index").toString();
        }
    }
    if(result.isEmpty() && !value.isEmpty()){
        result = udi();
        result = result.remove("/"+value);
    }
    d->m_parent_uid = result;
    return d->m_parent_uid;
}

QString WmiDevice::vendor() const
{
    return property(WmiDevicePrivate::getPropertyNameForVendor(d->m_type)).toString();
}

QString WmiDevice::product() const
{
    return property(WmiDevicePrivate::getPropertyNameForProduct(d->m_type)).toString();
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
    WmiQuery::Item item = d->sendQuery();

    QVariant result = item.getAllProperties()[key];
//    qDebug()<<"property"<<key<<result;
    return result;
}

const Solid::DeviceInterface::Type WmiDevice::type() const{
    return d->m_type;
}

QMap<QString, QVariant> WmiDevice::allProperties() const
{
    WmiQuery::Item item = d->sendQuery();

    return item.getAllProperties();
}

bool WmiDevice::propertyExists(const QString &key) const
{
    WmiQuery::Item item = d->sendQuery();
    const bool isEmpty = item.getProperty( key ).isValid();
    return isEmpty;
}

bool WmiDevice::queryDeviceInterface(const Solid::DeviceInterface::Type &type) const
{
    // Special cases not matching with WMI capabilities
    if (type==Solid::DeviceInterface::GenericInterface) {
        return true;
    }

    return d->interfaceList.contains(type);
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

    Q_FOREACH (const ChangeDescription &change, changes)
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

    Q_EMIT propertyChanged(result);
}

void WmiDevice::slotCondition(const QString &condition, const QString &reason)
{
    Q_EMIT conditionRaised(condition, reason);
}

#include "backends/wmi/wmidevice.moc"
