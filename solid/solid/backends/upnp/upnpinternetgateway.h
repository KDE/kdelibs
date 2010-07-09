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

#ifndef SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAY_H
#define SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAY_H

#include <solid/ifaces/internetgateway.h>

#include "upnpdeviceinterface.h"

#include <HAsyncOp>
#include <HDeviceProxy>
#include <HServiceProxy>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

    class UPnPInternetGateway : public Solid::Backends::UPnP::UPnPDeviceInterface, virtual public Solid::Ifaces::InternetGateway
    {
        Q_OBJECT
        Q_INTERFACES(Solid::Ifaces::InternetGateway)

        public:
            explicit UPnPInternetGateway(UPnPDevice* device);

            virtual ~UPnPInternetGateway();

            virtual void setEnabledForInternet(bool enabled) const;

            virtual Solid::InternetGateway::InternetStatus isEnabledForInternet() const;

            virtual void deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            virtual void addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol, 
                                        int newInternalPort, const QString newInternalClient, bool mappingEnabled, 
                                        const QString newPortMappingDescription, qlonglong newLeaseDuration);

            virtual QStringList currentConnections() const;

        private:
            Herqq::Upnp::HDeviceProxy* getDevice(const QString typePreffix, Herqq::Upnp::HDeviceProxies& devices) const;

            Herqq::Upnp::HServiceProxy* getWANConnectionService(Herqq::Upnp::HDeviceProxy* device) const;

        private slots:
            void setEnabledForInternetInvokeCallback(Herqq::Upnp::HAsyncOp invocationID);

            void deletePortMappingInvokeCallback(Herqq::Upnp::HAsyncOp invocationID);

            void addPortMappingInvokeCallback(Herqq::Upnp::HAsyncOp invocationID);    

        Q_SIGNALS:
            void enabledForInternet(bool enabled);

            void portMappingDeleted(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            void portMappingAdded(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol, 
                                  int newInternalPort, const QString newInternalClient, bool mappingEnabled, 
                                  const QString newPortMappingDescription, qlonglong newLeaseDuration);
    };

}
}
}

#endif // SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAY_H
