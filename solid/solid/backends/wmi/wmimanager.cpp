 /*  This file is part of the KDE project
    Copyright (C) 2005,2006 Kevin Ottens <ervin@kde.org>

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

#include "wmimanager.h"
#include "wmidevice.h"
#include "wmideviceinterface.h"

#include <QtCore/QDebug>

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


using namespace Solid::Backends::Wmi;

class Solid::Backends::Wmi::WmiManagerPrivate
{
public:
    WmiManagerPrivate()
        : failed(false)
        , pLoc(0)
        , pSvc(0)
        , pEnumerator(NULL)
    {
    
        //does this all look hacky?  yes...but it came straight from the MSDN example...
    
        HRESULT hres;

        hres =  CoInitializeEx( 0, COINIT_MULTITHREADED ); 
        if( FAILED(hres) )
        {
            qCritical() << "Failed to initialize COM library. " << "Error code = " << hres << endl;
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

    ~WmiManagerPrivate()
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
    
    bool failed;
    IWbemLocator *pLoc;
    IWbemServices *pSvc;
    IEnumWbemClassObject* pEnumerator;
};


WmiManager::WmiManager(QObject *parent)
    : DeviceManager(parent),  d(new WmiManagerPrivate())
{
    
}

WmiManager::~WmiManager()
{
    delete d;
}

QStringList WmiManager::allDevices()
{
    // QDBusReply<QStringList> reply = d->manager.call("GetAllDevices");

    // if (!reply.isValid())
    // {
        // qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        // return QStringList();
    // }

    // return reply;
    return QStringList();
}

bool WmiManager::deviceExists(const QString &udi)
{
    // QDBusReply<bool> reply = d->manager.call("DeviceExists", udi);

    // if (!reply.isValid())
    // {
        // qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        // return false;
    // }

    return false;
}

QStringList WmiManager::devicesFromQuery(const QString &parentUdi,
                                         Solid::DeviceInterface::Type type)
{
    if (!parentUdi.isEmpty())
    {
        QStringList result = findDeviceStringMatch("info.parent", parentUdi);

        if (type!=Solid::DeviceInterface::Unknown) {
            QStringList::Iterator it = result.begin();
            QStringList::ConstIterator end = result.end();

            for (; it!=end; ++it)
            {
                WmiDevice device(*it);

                if (!device.queryDeviceInterface(type)) {
                    result.erase(it);
                }
            }
        }

        return result;

    } else if (type!=Solid::DeviceInterface::Unknown) {
        return findDeviceByDeviceInterface(type);
    } else {
        return allDevices();
    }
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
    // QDBusReply<QStringList> reply = d->manager.call("FindDeviceStringMatch", key, value);

    // if (!reply.isValid())
    // {
        // qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
        // return QStringList();
    // }

    // return reply;
    return QStringList();
}

QStringList WmiManager::findDeviceByDeviceInterface(const Solid::DeviceInterface::Type &type)
{
    // QStringList cap_list = DeviceInterface::toStringList(type);
    // QStringList result;

    // foreach (const QString &cap, cap_list)
    // {
        // QDBusReply<QStringList> reply = d->manager.call("FindDeviceByCapability", cap);

        // if (!reply.isValid())
        // {
            // qWarning() << Q_FUNC_INFO << " error: " << reply.error().name() << endl;
            // return QStringList();
        // }
        // if ( cap == QLatin1String( "video4linux" ) )
        // {
            // QStringList foundDevices ( reply );
            // QStringList filtered;
            // foreach ( const QString &udi, foundDevices )
            // {
                // QDBusInterface device( "org.freedesktop.Wmi", udi, "org.freedesktop.Wmi.Device", QDBusConnection::systemBus() );
                // QDBusReply<QString> reply = device.call( "GetProperty", "video4linux.device" );
                // if (!reply.isValid())
                // {
                    // qWarning() << Q_FUNC_INFO << " error getting video4linux.device: " << reply.error().name() << endl;
                    // continue;
                // }
                // if ( !reply.value().contains( "video" ) )
                // {
                    // continue;
                // }
                // filtered.append( udi );
            // }
            // result += filtered;
        // }
        // else
        // {
            // result << reply;
        // }
    // }

    // return result;
    return QStringList();
}

void WmiManager::slotDeviceAdded(const QString &udi)
{
    emit deviceAdded(udi);
}

void WmiManager::slotDeviceRemoved(const QString &udi)
{
    emit deviceRemoved(udi);
}

#include "backends/wmi/wmimanager.moc"
