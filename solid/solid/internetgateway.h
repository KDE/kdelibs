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

#ifndef SOLID_INTERNETGATEWAY_H
#define SOLID_INTERNETGATEWAY_H

#include <deviceinterface.h>

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
            virtual ~InternetGateway();

            static Type deviceInterfaceType() { return DeviceInterface::InternetGateway; }

            QStringList currentConnections() const;

            void addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol,
                                int newInternalPort, const QString newInternalClient, bool mappingEnabled,
                                const QString newPortMappingDescription, int newLeaseDuration);

            void deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            bool isEnabledForInternet() const;

            void setEnabledForInternet(bool enabled);

        Q_SIGNALS:
            void portMappingAdded(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol,
                                  int newInternalPort, const QString newInternalClient, bool mappingEnabled,
                                  const QString newPortMappingDescription, int newLeaseDuration);

            void portMappingDeleted(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol);

            void enabledForInternet(bool enabled); // needed? [2]

        protected:
            InternetGateway(InternetGatewayPrivate& dd, QObject* backendObject);
    };

}
#endif // SOLID_INTERNETGATEWAY_H
