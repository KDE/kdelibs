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

#ifndef SOLID_IFACES_INTERNETGATEWAY_H
#define SOLID_IFACES_INTERNETGATEWAY_H

#include <QtCore/QStringList>

#include "deviceinterface.h"
#include "solid/internetgateway.h"

namespace Solid
{
namespace Ifaces
{

    class InternetGateway : virtual public Solid::Ifaces::DeviceInterface
    {
        public:
            virtual ~InternetGateway();

            virtual QStringList currentConnections() const = 0;

            virtual void addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol,
                                        int newInternalPort, const QString newInternalClient, bool mappingEnabled,
                                        const QString newPortMappingDescription, qlonglong newLeaseDuration) = 0;

            virtual void deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol) = 0;

            virtual Solid::InternetGateway::InternetStatus isEnabledForInternet() const = 0;

            virtual void setEnabledForInternet(bool enabled) const = 0;

        protected: //Q_SIGNALS:
            virtual void portMappingAdded(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol,
                                          int newInternalPort, const QString newInternalClient, bool mappingEnabled,
                                          const QString newPortMappingDescription, qlonglong newLeaseDuration) = 0;

            virtual void portMappingDeleted(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol) = 0;

            virtual void enabledForInternet(bool enabled) = 0; // needed?
    };

}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::InternetGateway, "org.kde.Solid.Ifaces.InternetGateway/0.1")

#endif // SOLID_IFACES_INTERNETGATEWAY_H
