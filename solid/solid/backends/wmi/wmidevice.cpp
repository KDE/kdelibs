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

#ifdef _DEBUG
# pragma comment(lib, "comsuppwd.lib")
#else
# pragma comment(lib, "comsuppw.lib")
#endif
# pragma comment(lib, "wbemuuid.lib")

#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>

# pragma comment(lib, "wbemuuid.lib")

#include <QtCore/QDebug>

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
        : parent(0)
        , failed(false)
        , pLoc(0)
        , pSvc(0)
        , pEnumerator(NULL)
    {
    
        //does this all look hacky?  yes...but it came straight from the MSDN example...
    
        HRESULT hres;

        hres =  CoInitializeEx( 0, COINIT_MULTITHREADED ); 
        if( FAILED(hres) )
        {
            qCritical() << "Failed to initialize COM library.  Error code = 0x" << hex << quint32(hres) << endl;
            failed = true;
        }
        if( !failed )
        {
            hres =  CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
                        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL );
                             
            if( FAILED(hres) )
            {
                qCritical() << "Failed to initialize security. " << "Error code = " << hres << endl;
                CoUninitialize();
                failed = true;
            }
        }
        if( !failed )
        {
            hres = CoCreateInstance( CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc );
            if (FAILED(hres))
            {
                qCritical() << "Failed to create IWbemLocator object. " << "Error code = " << hres << endl;
                CoUninitialize();
                failed = true;
            }
        }
        if( !failed )
        {
            hres = pLoc->ConnectServer( _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc );                              
            if( FAILED(hres) )
            {
                qCritical() << "Could not connect. Error code = " << hres << endl;
                pLoc->Release();
                CoUninitialize();
                failed = true;
            }
            else
                qDebug() << "Connected to ROOT\\CIMV2 WMI namespace" << endl;
        }
        
        if( !failed )
        {
            hres = CoSetProxyBlanket( pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );
            if( FAILED(hres) )
            {
                qCritical() << "Could not set proxy blanket. Error code = " << hres << endl;
                pSvc->Release();
                pLoc->Release();     
                CoUninitialize();
                failed = true;
            }
        }
    }

    ~WmiDevicePrivate()
    {
        pSvc->Release();
        pLoc->Release();     
        CoUninitialize();
    }
    
    
    QList<IWbemClassObject*> sendQuery( const QString &wql )
    {
        QList<IWbemClassObject*> retList;
        
        HRESULT hres;
        hres = pSvc->ExecQuery( bstr_t("WQL"), bstr_t( qPrintable( wql ) ),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);
    
        if( FAILED(hres) )
        {
            qDebug() << "Query with string \"" << wql << "\" failed. Error code = " << hres << endl;
        }
        else
        { 
            ULONG uReturn = 0;
       
            while( pEnumerator )
            {
                IWbemClassObject *pclsObj;
                hres = pEnumerator->Next( WBEM_INFINITE, 1, &pclsObj, &uReturn );

                if( !uReturn )
                    break;
                
                retList.append( pclsObj );
                
                //VARIANT vtProp;

                // Get the value of the Name property
                //hres = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
                //wcout << "Process Name : " << vtProp.bstrVal << endl;
                //VariantClear(&vtProp);
            }
        } 
    }
   
    bool isLegit() { return !failed; }
    
    WmiDevice *parent;
    bool failed;
    IWbemLocator *pLoc;
    IWbemServices *pSvc;
    IEnumWbemClassObject* pEnumerator;
};

Q_DECLARE_METATYPE(ChangeDescription)
Q_DECLARE_METATYPE(QList<ChangeDescription>)

WmiDevice::WmiDevice(const QString &udi)
    : Device(), d(new WmiDevicePrivate(udi))
{
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
    return QString();
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
    // QDBusMessage reply = d->device.call("GetProperty", key);

    // if (reply.type()!=QDBusMessage::ReplyMessage
      // && reply.errorName()!="org.freedesktop.Wmi.NoSuchProperty")
    // {
        // qWarning() << Q_FUNC_INFO << " error: " << reply.errorName()
                   // << ", " << reply.arguments().at(0).toString() << endl;
        // return QVariant();
    // }

    // return reply.arguments().at(0);
    return QVariant();
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
    // QDBusReply<bool> reply = d->device.call("PropertyExists", key);

    // if (!reply.isValid())
    // {
        // qDebug() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        // return false;
    // }

    // return reply;
    return false;
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

    // foreach (const QString &cap, cap_list)
    // {
        // QDBusReply<bool> reply = d->device.call("QueryCapability", cap);

        // if (!reply.isValid())
        // {
            // qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
            // return false;
        // }

        // if (reply) return reply;
    // }

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
    }

    emit propertyChanged(result);
}

void WmiDevice::slotCondition(const QString &condition, const QString &reason)
{
    emit conditionRaised(condition, reason);
}

#include "backends/wmi/wmidevice.moc"
