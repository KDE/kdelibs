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

#include "internetgateway.h"
#include "internetgateway_p.h"

#include "soliddefs_p.h"

#include <solid/ifaces/internetgateway.h>

namespace Solid
{

InternetGateway::InternetGateway(QObject* backendObject) :
    DeviceInterface(*new InternetGatewayPrivate(), backendObject)
{
    connect(backendObject,
            SIGNAL(portMappingAdded(const QString&, qint16, const NetworkProtocol&, qint16, const QString&)),
            this,
            SIGNAL(portMappingAdded(const QString&, qint16, const NetworkProtocol&, qint16, const QString&)));

    connect(backendObject,
            SIGNAL(portMappingDeleted(const QString&, qint16, const NetworkProtocol&)),
            this,
            SIGNAL(portMappingDeleted(const QString&, qint16, const NetworkProtocol&)));

    connect(backendObject,
            SIGNAL(enabledForInternet(bool)),
            this,
            SIGNAL(enabledForInternet(bool)));
}

InternetGateway::InternetGateway(InternetGatewayPrivate& dd, QObject* backendObject)
    : DeviceInterface(dd, backendObject)
{
    connect(backendObject,
            SIGNAL(portMappingAdded(const QString&, qint16, const NetworkProtocol&, qint16, const QString&)),
            this,
            SIGNAL(portMappingAdded(const QString&, qint16, const NetworkProtocol&, qint16, const QString&)));

    connect(backendObject,
            SIGNAL(portMappingDeleted(const QString&, qint16, const NetworkProtocol&)),
            this,
            SIGNAL(portMappingDeleted(const QString&, qint16, const NetworkProtocol&)));

    connect(backendObject,
            SIGNAL(enabledForInternet(bool)),
            this,
            SIGNAL(enabledForInternet(bool)));
}

InternetGateway::~InternetGateway()
{
}

void InternetGateway::requestCurrentConnections() const
{
    Q_D(const InternetGateway);

    SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(), requestCurrentConnections());
}

QStringList InternetGateway::currentConnections() const
{
    Q_D(const InternetGateway);

    return_SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(), QStringList(), currentConnections());
}

void InternetGateway::addPortMapping(const QString& remoteHost, qint16 externalPort, const NetworkProtocol& mappingProtocol,
                                     qint16 internalPort, const QString& internalClient)
{
    Q_D(const InternetGateway);

    SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(),
               addPortMapping(remoteHost, externalPort, mappingProtocol, internalPort, internalClient));
}

void InternetGateway::deletePortMapping(const QString& remoteHost, qint16 externalPort, const NetworkProtocol& mappingProtocol)
{
    Q_D(const InternetGateway);

    SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(),
               deletePortMapping(remoteHost, externalPort, mappingProtocol));
}

InternetGateway::InternetStatus InternetGateway::isEnabledForInternet() const
{
    Q_D(const InternetGateway);

    return_SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(), InternetGateway::UnknownStatus, isEnabledForInternet());
}

void InternetGateway::setEnabledForInternet(bool enabled)
{
    Q_D(const InternetGateway);

    SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(), setEnabledForInternet(enabled));
}

}
