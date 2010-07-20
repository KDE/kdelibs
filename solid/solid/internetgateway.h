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

#ifndef SOLID_INTERNETGATEWAY_H
#define SOLID_INTERNETGATEWAY_H

#include <solid/deviceinterface.h>

#include <solid/solid_export.h>
#include <solid/solidnamespace.h>

namespace Solid
{
    class InternetGatewayPrivate;

    class Device;

    class SOLID_EXPORT InternetGateway : public Solid::DeviceInterface
    {
        Q_OBJECT
        Q_DECLARE_PRIVATE(InternetGateway)
        friend class Device;

        private:
            explicit InternetGateway(QObject* backendObject);

        public:
            enum InternetStatus { InternetEnabled = 0, InternetDisabled = 1, UnknownStatus = 2 };
            
            virtual ~InternetGateway();

            static Type deviceInterfaceType() { return DeviceInterface::InternetGateway; }

            QStringList currentConnections() const;

            void addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol,
                                int newInternalPort, const QString newInternalClient, bool mappingEnabled,
                                const QString newPortMappingDescription, qlonglong newLeaseDuration);

            void deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            InternetStatus isEnabledForInternet() const;

            void setEnabledForInternet(bool enabled);

        Q_SIGNALS:
            void portMappingAdded(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol,
                                  int newInternalPort, const QString newInternalClient, bool mappingEnabled,
                                  const QString newPortMappingDescription, qlonglong newLeaseDuration);

            void portMappingDeleted(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            void enabledForInternet(bool enabled); // needed? [2]

        protected:
            InternetGateway(InternetGatewayPrivate& dd, QObject* backendObject);
    };

}
#endif // SOLID_INTERNETGATEWAY_H
