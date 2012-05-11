/*
    Copyright 2012 Patrick von Reth <vonreth@kde.org>
    Copyright 2005,2006 Kevin Ottens <ervin@kde.org>

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

#include "wmimanager.h"

#include <QtCore/QDebug>

#include "wmidevice.h"
#include "wmideviceinterface.h"
#include "wmiquery.h"


using namespace Solid::Backends::Wmi;

class Solid::Backends::Wmi::WmiManagerPrivate
{
public:
    WmiManagerPrivate()
    {
        update();
    }

    ~WmiManagerPrivate()
    {

    }

    void update(){
        init();
        m_deviceCache.clear();
    }

    void init(){
        m_volumes.clear();
        WmiQuery::ItemList items = WmiQuery::instance().sendQuery("SELECT * FROM Win32_LogicalDisk");
        foreach(const WmiQuery::Item &i,items){
            QString drive = i.getProperty("DeviceID").toString();
            WmiQuery::Item item = WmiDevice::win32LogicalDiskToDiskPartition(drive);
            m_volumes.insert(drive,item.getProperty("DeviceID").toString());
        }
    }

    QStringList allDevices(){
        if(m_deviceCache.isEmpty())
        {
            QStringList aList;
            foreach(const Solid::DeviceInterface::Type &dev, supportedInterfaces)
                aList<<findDeviceByDeviceInterface(dev);
            foreach(const QString &udi, aList)
            {
                if (!m_deviceCache.contains(udi))
                    m_deviceCache<< udi;
            }
        }
        return m_deviceCache;
    }


    QStringList findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
    {
        QStringList result;

        switch (type)
        {
        case Solid::DeviceInterface::Processor:
            result << WmiDevice::generateUDIList(type);
            break;
        case Solid::DeviceInterface::StorageAccess:
            result << WmiDevice::generateUDIList(type);
            break;
        case Solid::DeviceInterface::StorageDrive:
            result << WmiDevice::generateUDIList(type);
            break;
        case Solid::DeviceInterface::OpticalDrive:
            result << WmiDevice::generateUDIList(type);
            break;
        case Solid::DeviceInterface::StorageVolume:
            result << WmiDevice::generateUDIList(type);
            break;
        case Solid::DeviceInterface::OpticalDisc:
            result << WmiDevice::generateUDIList(type);
            break;
        case Solid::DeviceInterface::Battery:
            result << WmiDevice::generateUDIList(type);
            break;
        }

        return result;
    }

    WmiQuery::ItemList sendQuery( const QString &wql )
    {
		return WmiQuery::instance().sendQuery( wql );
    }

    QSet<Solid::DeviceInterface::Type> supportedInterfaces;

    QMap<QString,QString> m_volumes;
    QStringList m_deviceCache;
};


WmiManager::WmiManager(QObject *parent)
    : DeviceManager(parent),  d(new WmiManagerPrivate())
{
    d->supportedInterfaces << Solid::DeviceInterface::GenericInterface
                           << Solid::DeviceInterface::Processor
                           << Solid::DeviceInterface::Block
                           << Solid::DeviceInterface::StorageAccess
                           << Solid::DeviceInterface::StorageDrive
                           << Solid::DeviceInterface::OpticalDrive
                           << Solid::DeviceInterface::StorageVolume
                           << Solid::DeviceInterface::OpticalDisc
                           << Solid::DeviceInterface::Camera
                           << Solid::DeviceInterface::PortableMediaPlayer
                           << Solid::DeviceInterface::NetworkInterface
                           << Solid::DeviceInterface::AcAdapter
                           << Solid::DeviceInterface::Battery
                           << Solid::DeviceInterface::Button
                           << Solid::DeviceInterface::AudioInterface
                           << Solid::DeviceInterface::DvbInterface
                           << Solid::DeviceInterface::Video
                           << Solid::DeviceInterface::SerialInterface
                           << Solid::DeviceInterface::SmartCardReader;

    WmiQuery::instance().addDeviceListeners("SELECT * FROM Win32_VolumeChangeEvent",new WmiManager::WmiEventSink(this));

}

WmiManager::~WmiManager()
{
    delete d;
}

QString WmiManager::udiPrefix() const
{
    return QString(); //FIXME: We should probably use a prefix there... has to be tested on Windows
}

QSet<Solid::DeviceInterface::Type> WmiManager::supportedInterfaces() const
{
    return d->supportedInterfaces;
}

QStringList WmiManager::allDevices()
{

    return d->allDevices();
}

bool WmiManager::deviceExists(const QString &udi)
{
    return WmiDevice::exists(udi);
}


QStringList WmiManager::devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type)
{
    QStringList result;
    if (!parentUdi.isEmpty())
    {
        foreach(const QString &udi,d->allDevices()){
            WmiDevice device(udi);
            if(device.type() == type && device.parentUdi() == parentUdi ){
                result<<udi;
            }
        }

    } else if (type!=Solid::DeviceInterface::Unknown) {
            result<<findDeviceByDeviceInterface(type);
        } else {
            result<<allDevices();
        }
    return result;
}

QObject *WmiManager::createDevice(const QString &udi)
{
    if (deviceExists(udi)) {
        return new WmiDevice(udi);
    } else {
        return 0;
    }
}

QStringList WmiManager::findDeviceStringMatch(const QString &key, const QString &value)
{
    qDebug() << "has to be implemented" << key << value;
    QStringList result;

//    qDebug() << result;
    return result;
}

QStringList WmiManager::findDeviceByDeviceInterface(Solid::DeviceInterface::Type type)
{
    return d->findDeviceByDeviceInterface(type);
}

void WmiManager::slotDeviceAdded(const QString &udi)
{
    qDebug()<<"Device added"<<udi;
    emit deviceAdded(udi);
}

void WmiManager::slotDeviceRemoved(const QString &udi)
{
    qDebug()<<"Device removed"<<udi;
    emit deviceRemoved(udi);
}



WmiManager::WmiEventSink::WmiEventSink(WmiManager* parent):
    m_parent(parent),
    m_count(0)
{}

WmiManager::WmiEventSink::~WmiEventSink()
{}

ulong STDMETHODCALLTYPE WmiManager::WmiEventSink::AddRef()
{
    return InterlockedIncrement(&m_count);
}

ulong STDMETHODCALLTYPE WmiManager::WmiEventSink::Release()
{
    long lRef = InterlockedDecrement(&m_count);
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT  STDMETHODCALLTYPE WmiManager::WmiEventSink::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppv = (IWbemObjectSink *) this;
        AddRef();
        return WBEM_S_NO_ERROR;
    }
    else return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE WmiManager::WmiEventSink::Indicate(long lObjectCount,IWbemClassObject **apObjArray){
    for (long i = 0; i < lObjectCount; ++i)
    {
        WmiQuery::Item item( apObjArray[i]);
        QString drive = item.getProperty("DriveName").toString();
        //we need to use m_parent->d->m_volumes as a cache because if the device is removed I cant query it anymore
        ushort event = item.getProperty("EventType").toUInt();
        switch(event){
        case 2:
        {
            m_parent->d->update();
            m_parent->slotDeviceAdded( "/org/kde/solid/wmi/volume/"+ m_parent->d->m_volumes[drive]);
        }
            break;
        case 3:
        {
            m_parent->slotDeviceRemoved("/org/kde/solid/wmi/volume/"+ m_parent->d->m_volumes[drive]);
            m_parent->d->update();
        }
            break;
        case  4:
        {
            qDebug()<<"drive:"<<drive<<"docking";
        }
            break;
        case 1:
        {
            qDebug()<<"drive:"<<drive<<"config changed";
        }
            break;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT STDMETHODCALLTYPE WmiManager::WmiEventSink::SetStatus(long lFlags,HRESULT hResult,BSTR strParam,IWbemClassObject *pObjParam)
{
    return WBEM_S_NO_ERROR;
}


#include "backends/wmi/wmimanager.moc"
