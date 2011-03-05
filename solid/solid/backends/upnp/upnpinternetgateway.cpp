/*
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

#include <HUpnpCore/HDeviceInfo>
#include <HUpnpCore/HServiceId>
#include <HUpnpCore/HClientAction>
#include <HUpnpCore/HActionInfo>
#include <HUpnpCore/HActionArguments>
#include <HUpnpCore/HClientStateVariable>

#include <QtCore/QtDebug>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

UPnPInternetGateway::UPnPInternetGateway(UPnPDevice* device) :
    UPnPDeviceInterface(device),
    activeConnections(QStringList())
{
    numberOfConnections = 0;
}

UPnPInternetGateway::~UPnPInternetGateway()
{
}

void UPnPInternetGateway::setEnabledForInternet(bool enabled) const
{
    Herqq::Upnp::HClientDevices embeddedDevices = upnpDevice()->device()->embeddedDevices();
    Herqq::Upnp::HClientDevice* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);

    if (wanDevice) {
        Herqq::Upnp::HClientService* wanCommonIfaceConfigService = wanDevice->serviceById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANCommonInterfaceConfig"));
        if (wanCommonIfaceConfigService) {
            Herqq::Upnp::HClientAction* setEnabledForInternetAction = wanCommonIfaceConfigService->actions()[QString::fromLatin1("SetEnabledForInternet")];
            if (setEnabledForInternetAction) {
                Herqq::Upnp::HActionArguments inArgs = setEnabledForInternetAction->info().inputArguments();
                inArgs["NewEnabledForInternet"].setValue(enabled);

                connect(setEnabledForInternetAction,
                        SIGNAL(invokeComplete(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)),
                        this,
                        SLOT(setEnabledForInternetInvokeCallback(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)));

                qDebug() << "setEnabledForInternetAction begin invoke";

                Herqq::Upnp::HClientActionOp id = setEnabledForInternetAction->beginInvoke(inArgs);
            } else {
                qWarning() << "Unable to retrieve SetEnabledForInternet action for this device:" << upnpDevice()->udi();
                qWarning() << "Maybe the device doesn't implement, since it's an optional action. See the service description XML.";
            }
        } else {
            qWarning() << "Unable to retrieve WANCommonInterfaceConfig service for this device:" << upnpDevice()->udi();
        }
    } else {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
    }
}

void UPnPInternetGateway::setEnabledForInternetInvokeCallback(Herqq::Upnp::HClientAction *action, const Herqq::Upnp::HClientActionOp &invocationID)
{
    qDebug() << "setEnabledForInternetAction callback";

    Herqq::Upnp::HClientAction* setEnabledForInternetAction = action;

    if (invocationID.returnValue() == Herqq::Upnp::UpnpSuccess) {
        qDebug() << "setEnabledForInternetAction invocation successful";

        Herqq::Upnp::HActionArguments inArgs = setEnabledForInternetAction->info().inputArguments();
        bool enabled = inArgs["NewEnabledForInternet"].value().toBool();

        emit enabledForInternet(enabled);
    } else {
        qDebug() << "setEnabledForInternetAction invocation failed";
    }
}

Herqq::Upnp::HClientDevice* UPnPInternetGateway::getDevice(const QString typePreffix, Herqq::Upnp::HClientDevices& devices) const
{
    foreach(Herqq::Upnp::HClientDevice* device, devices) {
        QString deviceType = device->info().deviceType().toString(Herqq::Upnp::HResourceType::TypeSuffix |
                                                                        Herqq::Upnp::HResourceType::Version);
        if (deviceType.startsWith(typePreffix)) {
            return device;
        };
    }

    return 0;
}

Solid::InternetGateway::InternetStatus UPnPInternetGateway::isEnabledForInternet() const
{
    Herqq::Upnp::HClientDevices embeddedDevices = upnpDevice()->device()->embeddedDevices();
    Herqq::Upnp::HClientDevice* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);

    if (wanDevice) {
        Herqq::Upnp::HClientService* wanCommonIfaceConfigService = wanDevice->serviceById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANCommonInterfaceConfig"));
        if (wanCommonIfaceConfigService) {
            const Herqq::Upnp::HClientStateVariable* enabledForInternetVariable = wanCommonIfaceConfigService->stateVariables()[QString::fromLatin1("EnabledForInternet")];
            if (enabledForInternetVariable) {
                bool enabled = enabledForInternetVariable->value().toBool();

                return enabled ? Solid::InternetGateway::InternetEnabled : Solid::InternetGateway::InternetDisabled;
            } else {
                qWarning() << "Unable to retrieve EnabledForInternet state variable for this device:" << upnpDevice()->udi();
                qWarning() << "Maybe the device doesn't implement, since it's optional. See the service description XML.";
                return Solid::InternetGateway::UnknownStatus;
            }
        } else {
            qWarning() << "Unable to retrieve WANCommonInterfaceConfig service for this device:" << upnpDevice()->udi();
            return Solid::InternetGateway::UnknownStatus;
        }
    } else {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
        return Solid::InternetGateway::UnknownStatus;
    }
}

void UPnPInternetGateway::deletePortMapping(const QString& remoteHost, qint16 externalPort, const Solid::InternetGateway::NetworkProtocol& mappingProtocol)
{
    Herqq::Upnp::HClientDevices embeddedDevices = upnpDevice()->device()->embeddedDevices();
    Herqq::Upnp::HClientDevice* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);
    if (wanDevice) {
        Herqq::Upnp::HClientService* wanConnectionService = getWANConnectionService(wanDevice);
        if (wanConnectionService) {
            Herqq::Upnp::HClientAction* deletePortMappingAction = wanConnectionService->actions()[QString::fromLatin1("DeletePortMapping")];
            if (deletePortMappingAction) {
                Herqq::Upnp::HActionArguments inArgs = deletePortMappingAction->info().inputArguments();
                inArgs["NewRemoteHost"].setValue(remoteHost);
                inArgs["NewExternalPort"].setValue(externalPort);

                if (mappingProtocol == Solid::InternetGateway::TCP) {
                    inArgs["NewProtocol"].setValue(QString::fromLatin1("TCP"));
                } else {
                    inArgs["NewProtocol"].setValue(QString::fromLatin1("UDP"));
                }

                connect(deletePortMappingAction,
                        SIGNAL(invokeComplete(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)),
                        this,
                        SLOT(deletePortMappingInvokeCallback(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)));

                qDebug() << "deletePortMappingAction begin invoke";
                qDebug() << "inArgs:"
                         << inArgs["NewRemoteHost"].value().toString()
                         << inArgs["NewExternalPort"].value().toInt()
                         << inArgs["NewProtocol"].value().toString();

                Herqq::Upnp::HClientActionOp id = deletePortMappingAction->beginInvoke(inArgs);
            } else {
                qWarning() << "Unable to retrieve DeletePortMapping action for this device:" << upnpDevice()->udi();
            }
        } else {
            qWarning() << "Unable to retrieve WAN*Connection (* = {IP, PPP}) service for this device:" << upnpDevice()->udi();
        }
    } else {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
    }
}

void UPnPInternetGateway::deletePortMappingInvokeCallback(Herqq::Upnp::HClientAction *action, const Herqq::Upnp::HClientActionOp &invocationID)
{
    qDebug() << "deletePortMappingAction callback";

    Herqq::Upnp::HClientAction* deletePortMappingAction = action;

    if (invocationID.returnValue() == Herqq::Upnp::UpnpSuccess) {
        qDebug() << "deletePortMapping Action invocation successful" << invocationID.returnValue();

        Herqq::Upnp::HActionArguments inArgs = deletePortMappingAction->info().inputArguments();
        QString newRemoteHost = inArgs["NewRemoteHost"].value().toString();
        int newExternalPort = inArgs["NewExternalPort"].value().toInt();
        QString newProtocol = inArgs["NewProtocol"].value().toString();

        Solid::InternetGateway::NetworkProtocol protocol;
        if (newProtocol == QString::fromLatin1("TCP")) {
            protocol = Solid::InternetGateway::TCP;
        } else {
            protocol = Solid::InternetGateway::UDP;
        }

        emit portMappingDeleted(newRemoteHost, newExternalPort, protocol);
    } else {
        qDebug() << "deletePortMapping Action invocation failed";
    }
}

Herqq::Upnp::HClientService* UPnPInternetGateway::getWANConnectionService(Herqq::Upnp::HClientDevice* device) const
{
    Herqq::Upnp::HClientDevices embeddedDevices = device->embeddedDevices();
    Herqq::Upnp::HClientDevice* wanConnectionDevice = getDevice(QString::fromLatin1("WANConnectionDevice"), embeddedDevices);

    Herqq::Upnp::HClientService* service = 0;

    service = wanConnectionDevice->serviceById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANPPPConnection"));

    if (service)
        return service;

    service = wanConnectionDevice->serviceById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANIPConnection"));

    return service;
}

void UPnPInternetGateway::addPortMapping(const QString& remoteHost, qint16 externalPort, const Solid::InternetGateway::NetworkProtocol& mappingProtocol,
                                         qint16 internalPort, const QString& internalClient)
{
    Herqq::Upnp::HClientDevices embeddedDevices = upnpDevice()->device()->embeddedDevices();
    Herqq::Upnp::HClientDevice* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);
    if (wanDevice) {
        Herqq::Upnp::HClientService* wanConnectionService = getWANConnectionService(wanDevice);
        if (wanConnectionService) {
            Herqq::Upnp::HClientAction* addPortMappingAction = wanConnectionService->actions()[QString::fromLatin1("AddPortMapping")];
            if (addPortMappingAction) {
                Herqq::Upnp::HActionArguments inArgs = addPortMappingAction->info().inputArguments();
                inArgs["NewRemoteHost"].setValue(remoteHost);
                inArgs["NewExternalPort"].setValue(externalPort);

                if (mappingProtocol == Solid::InternetGateway::TCP) {
                    inArgs["NewProtocol"].setValue(QString::fromLatin1("TCP"));
                } else {
                    inArgs["NewProtocol"].setValue(QString::fromLatin1("UDP"));
                }

                inArgs["NewInternalPort"].setValue(internalPort);
                inArgs["NewInternalClient"].setValue(internalClient);
                inArgs["NewEnabled"].setValue(true);
                inArgs["NewPortMappingDescription"].setValue(QString());
                inArgs["NewLeaseDuration"].setValue(0);

                connect(addPortMappingAction,
                        SIGNAL(invokeComplete(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)),
                        this,
                        SLOT(addPortMappingInvokeCallback(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)));

                qDebug() << "addPortMappingAction begin invoke";
                qDebug() << "inArgs:"
                         << inArgs["NewRemoteHost"].value().toString()
                         << inArgs["NewExternalPort"].value().toInt()
                         << inArgs["NewProtocol"].value().toString()
                         << inArgs["NewInternalPort"].value().toInt()
                         << inArgs["NewInternalClient"].value().toString()
                         << inArgs["NewEnabled"].value().toBool()
                         << inArgs["NewPortMappingDescription"].value().toString()
                         << inArgs["NewLeaseDuration"].value().toLongLong();

                Herqq::Upnp::HClientActionOp id = addPortMappingAction->beginInvoke(inArgs);
            } else {
                qWarning() << "Unable to retrieve AddPortMapping action for this device:" << upnpDevice()->udi();
            }
        } else {
            qWarning() << "Unable to retrieve WAN*Connection (* = {IP, PPP}) service for this device:" << upnpDevice()->udi();
        }
    } else {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
    }
}

void UPnPInternetGateway::addPortMappingInvokeCallback(Herqq::Upnp::HClientAction *action, const Herqq::Upnp::HClientActionOp &invocationID)
{
    qDebug() << "addPortMappingAction callback";

    Herqq::Upnp::HClientAction* addPortMappingAction = action;

    if (invocationID.returnValue() == Herqq::Upnp::UpnpSuccess) {
        qDebug() << "addPortMapping Action invocation successful" << invocationID.returnValue();

        Herqq::Upnp::HActionArguments inArgs = addPortMappingAction->info().inputArguments();
        QString newRemoteHost = inArgs["NewRemoteHost"].value().toString();
        int newExternalPort = inArgs["NewExternalPort"].value().toInt();
        QString newProtocol = inArgs["NewProtocol"].value().toString();

        Solid::InternetGateway::NetworkProtocol protocol;
        if (newProtocol == QString::fromLatin1("TCP")) {
            protocol = Solid::InternetGateway::TCP;
        } else {
            protocol = Solid::InternetGateway::UDP;
        }

        int newInternalPort = inArgs["NewInternalPort"].value().toInt();
        QString newInternalClient = inArgs["NewInternalClient"].value().toString();

        emit portMappingAdded(newRemoteHost, newExternalPort, protocol, newInternalPort, newInternalClient);
    } else {
        qDebug() << "addPortMapping Action invocation failed" << invocationID.returnValue();
    }
}

int UPnPInternetGateway::getNumberOfActiveConnections()
{
    Herqq::Upnp::HClientDevices embeddedDevices = upnpDevice()->device()->embeddedDevices();
    Herqq::Upnp::HClientDevice* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);

    if (wanDevice) {
        Herqq::Upnp::HClientService* wanCommonIfaceConfigService = wanDevice->serviceById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANCommonInterfaceConfig"));
        if (wanCommonIfaceConfigService) {
            const Herqq::Upnp::HClientStateVariable* numberOfActiveConnections = wanCommonIfaceConfigService->stateVariables()[QString::fromLatin1("NumberOfActiveConnections")];
            if (numberOfActiveConnections) {
                int number = numberOfActiveConnections->value().toInt();
                numberOfConnections = number;
                return number;
            } else {
                qWarning() << "Unable to retrieve NumberOfActiveConnections state variable for this device:" << upnpDevice()->udi();
                qWarning() << "Maybe the device doesn't implement, since it's optional. See the service description XML.";
                return -1;
            }
        } else {
            qWarning() << "Unable to retrieve WANCommonInterfaceConfig service for this device:" << upnpDevice()->udi();
            return -1;
        }
    } else {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
        return -1;
    }
}

void UPnPInternetGateway::requestCurrentConnections()
{
    Herqq::Upnp::HClientDevices embeddedDevices = upnpDevice()->device()->embeddedDevices();
    Herqq::Upnp::HClientDevice* wanDevice = getDevice(QString::fromLatin1("WANDevice"), embeddedDevices);

    if (wanDevice) {
        Herqq::Upnp::HClientService* wanCommonIfaceConfigService = wanDevice->serviceById(Herqq::Upnp::HServiceId("urn:upnp-org:serviceId:WANCommonInterfaceConfig"));
        if (wanCommonIfaceConfigService) {
            Herqq::Upnp::HClientAction* getActiveConnectionAction = wanCommonIfaceConfigService->actions()[QString::fromLatin1("GetActiveConnection")];
            if (getActiveConnectionAction) {
                int numberOfActiveConnections = getNumberOfActiveConnections();
                if (numberOfActiveConnections > 0) {
                    connect(getActiveConnectionAction,
                            SIGNAL(invokeComplete(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)),
                            this,
                            SLOT(getActiveConnectionActionInvokeCallback(Herqq::Upnp::HClientAction *, const Herqq::Upnp::HClientActionOp &)));

                    for (int i = 0; i < numberOfActiveConnections; ++i) {
                        Herqq::Upnp::HActionArguments inArgs = getActiveConnectionAction->info().inputArguments();
                        inArgs["NewActiveConnectionIndex"].setValue(i);

                        qDebug() << "getActiveConnectionAction begin invoke" << i;
                        Herqq::Upnp::HClientActionOp id = getActiveConnectionAction->beginInvoke(inArgs);
                    }
                }
            } else {
                qWarning() << "Unable to retrieve GetActiveConnection action for this device:" << upnpDevice()->udi();
                qWarning() << "Maybe the device doesn't implement, since it's an optional action. See the service description XML.";
            }
        } else {
            qWarning() << "Unable to retrieve WANCommonInterfaceConfig service for this device:" << upnpDevice()->udi();
        }
    } else {
        qWarning() << "Unable to retrieve WANDevice for this device:" << upnpDevice()->udi();
    }
}

void UPnPInternetGateway::getActiveConnectionActionInvokeCallback(Herqq::Upnp::HClientAction *action, const Herqq::Upnp::HClientActionOp &invocationID)
{
    qDebug() << "getActiveConnectionAction callback";
    Herqq::Upnp::HActionArguments outArgs = invocationID.outputArguments();

    Herqq::Upnp::HClientAction* getActiveConnectionAction = action;

    if (invocationID.returnValue() == Herqq::Upnp::UpnpSuccess) {
        qDebug() << "getActiveConnection Action invocation successful" << invocationID.returnValue();
        QString conn = outArgs["ActiveConnectionDeviceContainer"].value().toString();
        activeConnections << conn;
    } else {
        qDebug() << "getActiveConnection Action invocation failed";
    }

    if (activeConnections.count() == numberOfConnections) {
        emit currentConnectionsDataIsReady(activeConnections);
    }
}

QStringList UPnPInternetGateway::currentConnections() const
{
    return activeConnections;
}

}
}
}
