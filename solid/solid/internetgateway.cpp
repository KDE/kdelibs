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
            SIGNAL(portMappingAdded(const QString, int, const QString, int, const QString, bool, const QString, int)),
            this,
            SIGNAL(portMappingAdded(const QString, int, const QString, int, const QString, bool, const QString, int)));
    
    connect(backendObject,
            SIGNAL(portMappingDeleted(const QString, int, const QString)),
            this,
            SIGNAL(portMappingDeleted(const QString, int, const QString)));

    connect(backendObject,
            SIGNAL(enabledForInternet(bool)),
            this,
            SIGNAL(enabledForInternet(bool)));
}

InternetGateway::InternetGateway(InternetGatewayPrivate& dd, QObject* backendObject) 
    : DeviceInterface(dd, backendObject)
{
    connect(backendObject, 
            SIGNAL(portMappingAdded(const QString, int, const QString, int, const QString, bool, const QString, int)),
            this,
            SIGNAL(portMappingAdded(const QString, int, const QString, int, const QString, bool, const QString, int)));
    
    connect(backendObject,
            SIGNAL(portMappingDeleted(const QString, int, const QString)),
            this,
            SIGNAL(portMappingDeleted(const QString, int, const QString)));

    connect(backendObject,
            SIGNAL(enabledForInternet(bool)),
            this,
            SIGNAL(enabledForInternet(bool)));
}

InternetGateway::~InternetGateway()
{
}

QStringList InternetGateway::currentConnections() const
{
    Q_D(const InternetGateway);

    return_SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(), QStringList(), currentConnections());
}

void InternetGateway::addPortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol, 
                                     int newInternalPort, const QString newInternalClient, bool mappingEnabled, 
                                     const QString newPortMappingDescription, qlonglong newLeaseDuration)
{
    Q_D(const InternetGateway);
    
    SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(), 
               addPortMapping(newRemoteHost, newExternalPort, mappingProtocol, newInternalPort, newInternalClient,
                              mappingEnabled, newPortMappingDescription, newLeaseDuration));
}

void InternetGateway::deletePortMapping(const QString newRemoteHost, int newExternalPort, const QString mappingProtocol)
{
    Q_D(const InternetGateway);
    
    SOLID_CALL(Ifaces::InternetGateway*, d->backendObject(), 
               deletePortMapping(newRemoteHost, newExternalPort, mappingProtocol));
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