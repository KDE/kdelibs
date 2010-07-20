/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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

#include "upnpinternetgateway.h"

#include <HDeviceInfo>
#include <HServiceId>
#include <HAction>
#include <HActionArguments>
#include <HStateVariable>

#include <QtCore/QtDebug>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

UPnPInternetGateway::UPnPInternetGateway(UPnPDevice* device) :
    UPnPDeviceInterface(device)
{
}

UPnPInternetGateway::~UPnPInternetGateway()
{
}

void UPnPInternetGateway::setEnabledForInternet(bool enabled) const
{
    Herqq::Upnp::HDeviceProxies embeddedDevices = upnpDevice()->device()->embeddedProxyDevices();
    Herqq::Upnp::HDeviceProxy* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);
    
    if (wanDevice)
    {
        Herqq::Upnp::HServiceProxy* wanCommonIfaceConfigService = wanDevice->serviceProxyById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANCommonInterfaceConfig"));
        if (wanCommonIfaceConfigService)
        {
            Herqq::Upnp::HAction* setEnabledForInternetAction = wanCommonIfaceConfigService->actionByName(QString::fromLatin1("SetEnabledForInternet"));
            if (setEnabledForInternetAction)
            {
                Herqq::Upnp::HActionArguments inArgs = setEnabledForInternetAction->inputArguments();
                inArgs["NewEnabledForInternet"]->setValue(enabled);

                connect(setEnabledForInternetAction, 
                        SIGNAL(invokeComplete(Herqq::Upnp::HAsyncOp)),
                        this,
                        SLOT(setEnabledForInternetInvokeCallback(Herqq::Upnp::HAsyncOp)));
                
                qDebug() << "setEnabledForInternetAction begin invoke";
                
                Herqq::Upnp::HAsyncOp id = setEnabledForInternetAction->beginInvoke(inArgs);
                id.setUserData(reinterpret_cast<void*>(setEnabledForInternetAction));
            }
            else
            {
                qWarning() << "Unable to retrieve SetEnabledForInternet action for this device:" << upnpDevice()->udi();
                qWarning() << "Maybe the device doesn't implement, since it's an optional action. See the service description XML.";
            }
        }
        else
        {
            qWarning() << "Unable to retrieve WANCommonInterfaceConfig service for this device:" << upnpDevice()->udi();
        }
    }
    else
    {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
    }
}

void UPnPInternetGateway::setEnabledForInternetInvokeCallback(Herqq::Upnp::HAsyncOp invocationID)
{
    qDebug() << "setEnabledForInternetAction callback";

    Herqq::Upnp::HAction* setEnabledForInternetAction = reinterpret_cast<Herqq::Upnp::HAction*>(invocationID.userData());

    bool wait = setEnabledForInternetAction->waitForInvoke(&invocationID);
    
    if (wait)
    {
        qDebug() << "setEnabledForInternetAction invocation successful";
    
        Herqq::Upnp::HActionArguments inArgs = setEnabledForInternetAction->inputArguments();
        bool enabled = inArgs["NewEnabledForInternet"]->value().toBool();

        emit enabledForInternet(enabled);
    }
    else
    {
        qDebug() << "setEnabledForInternetAction invocation failed";
    }
}

Herqq::Upnp::HDeviceProxy* UPnPInternetGateway::getDevice(const QString typePreffix, Herqq::Upnp::HDeviceProxies& devices) const
{
    foreach(Herqq::Upnp::HDeviceProxy* device, devices)
    {
        QString deviceType = device->deviceInfo().deviceType().toString(Herqq::Upnp::HResourceType::TypeSuffix | 
                                                                        Herqq::Upnp::HResourceType::Version);
        if (deviceType.startsWith(typePreffix))
        {
            return device;
        };
    }

    return 0;
}

Solid::InternetGateway::InternetStatus UPnPInternetGateway::isEnabledForInternet() const
{
    Herqq::Upnp::HDeviceProxies embeddedDevices = upnpDevice()->device()->embeddedProxyDevices();
    Herqq::Upnp::HDeviceProxy* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);
    
    if (wanDevice)
    {
        Herqq::Upnp::HServiceProxy* wanCommonIfaceConfigService = wanDevice->serviceProxyById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANCommonInterfaceConfig"));
        if (wanCommonIfaceConfigService)
        {
            Herqq::Upnp::HStateVariable* enabledForInternetVariable = wanCommonIfaceConfigService->stateVariableByName(QString::fromLatin1("EnabledForInternet"));
            if (enabledForInternetVariable)
            {
                bool enabled = enabledForInternetVariable->value().toBool();
                
                return enabled ? Solid::InternetGateway::InternetEnabled : Solid::InternetGateway::InternetDisabled;                
            }
            else
            {
                qWarning() << "Unable to retrieve EnabledForInternet state variable for this device:" << upnpDevice()->udi();                
                qWarning() << "Maybe the device doesn't implement, since it's optional. See the service description XML.";
                return Solid::InternetGateway::UnknownStatus;
            }
        }
        else
        {
            qWarning() << "Unable to retrieve WANCommonInterfaceConfig service for this device:" << upnpDevice()->udi();            
            return Solid::InternetGateway::UnknownStatus;
        }
    }
    else
    {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();        
        return Solid::InternetGateway::UnknownStatus;
    }
}

void UPnPInternetGateway::deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol)
{
    Herqq::Upnp::HDeviceProxies embeddedDevices = upnpDevice()->device()->embeddedProxyDevices();
    Herqq::Upnp::HDeviceProxy* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);
    if (wanDevice)
    {
        Herqq::Upnp::HServiceProxy* wanConnectionService = getWANConnectionService(wanDevice);
        if (wanConnectionService)
        {
            Herqq::Upnp::HAction* deletePortMappingAction = wanConnectionService->actionByName(QString::fromLatin1("DeletePortMapping"));
            if (deletePortMappingAction)
            {
                Herqq::Upnp::HActionArguments inArgs = deletePortMappingAction->inputArguments();
                inArgs["NewRemoteHost"]->setValue(newRemoteHost);
                inArgs["NewExternalPort"]->setValue(newExternalPort);
                inArgs["NewProtocol"]->setValue(mappingProtocol);

                connect(deletePortMappingAction, 
                        SIGNAL(invokeComplete(Herqq::Upnp::HAsyncOp)),
                        this,
                        SLOT(deletePortMappingInvokeCallback(Herqq::Upnp::HAsyncOp)));
                
                qDebug() << "deletePortMappingAction begin invoke";
                qDebug() << "inArgs:" 
                         << inArgs["NewRemoteHost"]->value().toString()
                         << inArgs["NewExternalPort"]->value().toInt()
                         << inArgs["NewProtocol"]->value().toString();
                
                Herqq::Upnp::HAsyncOp id = deletePortMappingAction->beginInvoke(inArgs);
                id.setUserData(reinterpret_cast<void*>(deletePortMappingAction));
            }
            else
            {
                qWarning() << "Unable to retrieve DeletePortMapping action for this device:" << upnpDevice()->udi();
            }
        }
        else
        {
            qWarning() << "Unable to retrieve WAN*Connection (* = {IP, PPP}) service for this device:" << upnpDevice()->udi();
        }
    }
    else
    {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
    }    
}

void UPnPInternetGateway::deletePortMappingInvokeCallback(Herqq::Upnp::HAsyncOp invocationID)
{
    qDebug() << "deletePortMappingAction callback";

    Herqq::Upnp::HAction* deletePortMappingAction = reinterpret_cast<Herqq::Upnp::HAction*>(invocationID.userData());

    bool wait = deletePortMappingAction->waitForInvoke(&invocationID);
    
    if (wait)
    {
        qDebug() << "deletePortMapping Action invocation successful" << invocationID.waitCode() << invocationID.returnValue();
    
        Herqq::Upnp::HActionArguments inArgs = deletePortMappingAction->inputArguments();
        QString newRemoteHost = inArgs["NewRemoteHost"]->value().toString();
        int newExternalPort = inArgs["NewExternalPort"]->value().toInt();
        QString newProtocol = inArgs["NewProtocol"]->value().toString();

        emit portMappingDeleted(newRemoteHost, newExternalPort, newProtocol);
    }
    else 
    {
        qDebug() << "deletePortMapping Action invocation failed";
    }
}

Herqq::Upnp::HServiceProxy* UPnPInternetGateway::getWANConnectionService(Herqq::Upnp::HDeviceProxy* device) const
{
    Herqq::Upnp::HDeviceProxies embeddedDevices = device->embeddedProxyDevices();
    Herqq::Upnp::HDeviceProxy* wanConnectionDevice = getDevice(QString::fromLatin1("WANConnectionDevice"), embeddedDevices);

    Herqq::Upnp::HServiceProxy* service = 0;
    
    service = wanConnectionDevice->serviceProxyById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANPPPConnection"));
    
    if (service)
        return service;
    
    service = wanConnectionDevice->serviceProxyById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANIPConnection"));
    
    return service;
}

void UPnPInternetGateway::addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol, 
                                         int newInternalPort, const QString newInternalClient, bool mappingEnabled, 
                                         const QString newPortMappingDescription, qlonglong newLeaseDuration)
{
    Herqq::Upnp::HDeviceProxies embeddedDevices = upnpDevice()->device()->embeddedProxyDevices();
    Herqq::Upnp::HDeviceProxy* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);
    if (wanDevice)
    {
        Herqq::Upnp::HServiceProxy* wanConnectionService = getWANConnectionService(wanDevice);
        if (wanConnectionService)
        {
            Herqq::Upnp::HAction* addPortMappingAction = wanConnectionService->actionByName(QString::fromLatin1("AddPortMapping"));
            if (addPortMappingAction)
            {
                Herqq::Upnp::HActionArguments inArgs = addPortMappingAction->inputArguments();
                inArgs["NewRemoteHost"]->setValue(newRemoteHost);
                inArgs["NewExternalPort"]->setValue(newExternalPort);
                inArgs["NewProtocol"]->setValue(mappingProtocol);
                inArgs["NewInternalPort"]->setValue(newInternalPort);
                inArgs["NewInternalClient"]->setValue(newInternalClient);
                inArgs["NewEnabled"]->setValue(mappingEnabled);
                inArgs["NewPortMappingDescription"]->setValue(newPortMappingDescription);
                inArgs["NewLeaseDuration"]->setValue(newLeaseDuration);

                connect(addPortMappingAction, 
                        SIGNAL(invokeComplete(Herqq::Upnp::HAsyncOp)),
                        this,
                        SLOT(addPortMappingInvokeCallback(Herqq::Upnp::HAsyncOp)));
                
                qDebug() << "addPortMappingAction begin invoke";
                qDebug() << "inArgs:"
                         << inArgs["NewRemoteHost"]->value().toString()
                         << inArgs["NewExternalPort"]->value().toInt()
                         << inArgs["NewProtocol"]->value().toString()
                         << inArgs["NewInternalPort"]->value().toInt()
                         << inArgs["NewInternalClient"]->value().toString()
                         << inArgs["NewEnabled"]->value().toBool()
                         << inArgs["NewPortMappingDescription"]->value().toString()
                         << inArgs["NewLeaseDuration"]->value().toLongLong();

                Herqq::Upnp::HAsyncOp id = addPortMappingAction->beginInvoke(inArgs);
                id.setUserData(reinterpret_cast<void*>(addPortMappingAction));
            }
            else
            {
                qWarning() << "Unable to retrieve AddPortMapping action for this device:" << upnpDevice()->udi();
            }
        }
        else
        {
            qWarning() << "Unable to retrieve WAN*Connection (* = {IP, PPP}) service for this device:" << upnpDevice()->udi();
        }
    }
    else
    {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
    } 
}

void UPnPInternetGateway::addPortMappingInvokeCallback(Herqq::Upnp::HAsyncOp invocationID)
{
    qDebug() << "addPortMappingAction callback";

    Herqq::Upnp::HAction* addPortMappingAction = reinterpret_cast<Herqq::Upnp::HAction*>(invocationID.userData());

    bool wait = addPortMappingAction->waitForInvoke(&invocationID);
    
    if (wait)
    {
        qDebug() << "addPortMapping Action invocation successful" << invocationID.waitCode() << invocationID.returnValue();        
    
        Herqq::Upnp::HActionArguments inArgs = addPortMappingAction->inputArguments();
        QString newRemoteHost = inArgs["NewRemoteHost"]->value().toString();
        int newExternalPort = inArgs["NewExternalPort"]->value().toInt();
        QString newProtocol = inArgs["NewProtocol"]->value().toString();
        int newInternalPort = inArgs["NewInternalPort"]->value().toInt();
        QString newInternalClient = inArgs["NewInternalClient"]->value().toString();
        bool mappingEnabled = inArgs["NewEnabled"]->value().toBool();
        QString newPortMappingDescription = inArgs["NewPortMappingDescription"]->value().toString();
        qlonglong newLeaseDuration = inArgs["NewLeaseDuration"]->value().toLongLong();

        emit portMappingAdded(newRemoteHost, newExternalPort, newProtocol, newInternalPort, newInternalClient,
                              mappingEnabled, newPortMappingDescription, newLeaseDuration);
    }
    else 
    {
        qDebug() << "addPortMapping Action invocation failed" << invocationID.waitCode() << invocationID.returnValue();
    }
}

QStringList UPnPInternetGateway::currentConnections() const
{
    return QStringList();
}

}
}
}