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

#include "upnpmediaserver.h"

#include <HDeviceInfo>
#include <HUdn>

#include <QtCore/QUrl>
#include <QtCore/QTimer>

namespace Solid
{
namespace Backends
{
namespace UPnP
{

UPnPMediaServer::UPnPMediaServer(UPnPDevice* device) :
    UPnPDeviceInterface(device)
{
}

UPnPMediaServer::~UPnPMediaServer()
{
}

bool UPnPMediaServer::isAccessible() const
{
    return upnpDevice()->isValid();
}

QString UPnPMediaServer::filePath() const
{
    if (isAccessible())
    {
        QString scheme = "upnp-ms:";
        QString udn = upnpDevice()->device()->deviceInfo().udn().toString();
        QString uuid = udn.mid(5); //udn without the uuid: preffix

        return (scheme + QString::fromLatin1("//") + uuid);
    }

    return QString();
}

bool UPnPMediaServer::setup()
{
    QTimer::singleShot(0, this, SLOT(onSetupTimeout()));

    return true;
}

bool UPnPMediaServer::teardown()
{
    QTimer::singleShot(0, this, SLOT(onTeardownTimeout()));

    return true;
}

void UPnPMediaServer::onSetupTimeout()
{
    emit setupDone(Solid::NoError, QVariant(), upnpDevice()->udi());
}

void UPnPMediaServer::onTeardownTimeout()
{
    emit teardownDone(Solid::NoError, QVariant(), upnpDevice()->udi());
}

}
}
}