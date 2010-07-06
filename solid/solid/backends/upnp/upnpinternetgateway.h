/*
   This file is part of the KDE project

   Copyright 2010 Paulo Romulo Alves Barros <paulo.romulo@kdemail.net>

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

#ifndef SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAY_H
#define SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAY_H

#include <solid/ifaces/internetgateway.h>

#include "upnpdeviceinterface.h"

#include <HAsyncOp>
#include <HDeviceProxy>

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

            virtual bool isEnabledForInternet() const;

            virtual void deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            virtual void addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol, 
                                        int newInternalPort, const QString newInternalClient, bool mappingEnabled, 
                                        const QString newPortMappingDescription, int newLeaseDuration);

            virtual QStringList currentConnections() const;

        private:
            Herqq::Upnp::HDeviceProxy* getWANDevice(Herqq::Upnp::HDeviceProxies& devices) const;

        private slots:
            void setEnabledForInternetInvokeCallback(Herqq::Upnp::HAsyncOp invocationID);

        Q_SIGNALS:
            void enabledForInternet(bool enabled);

            void portMappingDeleted(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            void portMappingAdded(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol, 
                                  int newInternalPort, const QString newInternalClient, bool mappingEnabled, 
                                  const QString newPortMappingDescription, int newLeaseDuration);
    };

}
}
}

#endif // SOLID_BACKENDS_UPNP_UPNPINTERNETGATEWAY_H
