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
#include <HServiceProxy>
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
    Herqq::Upnp::HDeviceProxy* wanDevice = getWANDevice(embeddedDevices);
    
    if (wanDevice)
    {
        Herqq::Upnp::HServiceProxy* wanCommonIfaceConfigService = wanDevice->serviceProxyById(Herqq::Upnp::HServiceId("urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1"));
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
    wait ? qDebug() << "setEnabledForInternetAction invocation successful" : qDebug() << "setEnabledForInternetAction invocation failed";
    
    Herqq::Upnp::HActionArguments inArgs = setEnabledForInternetAction->inputArguments();
    bool enabled = inArgs["NewEnabledForInternet"]->value().toBool();

    emit enabledForInternet(enabled);
}

Herqq::Upnp::HDeviceProxy* UPnPInternetGateway::getWANDevice(Herqq::Upnp::HDeviceProxies& devices) const
{
    foreach(Herqq::Upnp::HDeviceProxy* device, devices)
    {
        QString deviceType = device->deviceInfo().deviceType().toString(Herqq::Upnp::HResourceType::TypeSuffix | 
                                                                        Herqq::Upnp::HResourceType::Version);
        if (deviceType.startsWith(QString::fromLatin1("WANDevice")))
        {
            return device;
        };
    }

    return 0;
}

bool UPnPInternetGateway::isEnabledForInternet() const
{
    Herqq::Upnp::HDeviceProxies embeddedDevices = upnpDevice()->device()->embeddedProxyDevices();
    Herqq::Upnp::HDeviceProxy* wanDevice = getWANDevice(embeddedDevices);
    
    if (wanDevice)
    {
        Herqq::Upnp::HServiceProxy* wanCommonIfaceConfigService = wanDevice->serviceProxyById(Herqq::Upnp::HServiceId("urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1"));
        if (wanCommonIfaceConfigService)
        {
            Herqq::Upnp::HStateVariable* enabledForInternetVariable = wanCommonIfaceConfigService->stateVariableByName(QString::fromLatin1("EnabledForInternet"));
            if (enabledForInternetVariable)
            {
                return enabledForInternetVariable->value().toBool();
            }
            else
            {
                qWarning() << "Unable to retrieve EnabledForInternet state variable for this device:" << upnpDevice()->udi();
                return false;
            }
        }
        else
        {
            qWarning() << "Unable to retrieve WANCommonInterfaceConfig service for this device:" << upnpDevice()->udi();
            return false;
        }
    }
    else
    {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
        return false;
    }
}

void UPnPInternetGateway::deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol)
{
    Q_UNUSED(newRemoteHost);
    Q_UNUSED(newExternalPort);
    Q_UNUSED(mappingProtocol);
}

void UPnPInternetGateway::addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol, 
                                         int newInternalPort, const QString newInternalClient, bool mappingEnabled, 
                                         const QString newPortMappingDescription, int newLeaseDuration)
{
    Q_UNUSED(newRemoteHost);
    Q_UNUSED(newExternalPort);
    Q_UNUSED(mappingProtocol);
    Q_UNUSED(newInternalPort);
    Q_UNUSED(newInternalClient);
    Q_UNUSED(mappingEnabled);
    Q_UNUSED(newPortMappingDescription);
    Q_UNUSED(newLeaseDuration);
}

QStringList UPnPInternetGateway::currentConnections() const
{
    return QStringList();
}

}
}
}