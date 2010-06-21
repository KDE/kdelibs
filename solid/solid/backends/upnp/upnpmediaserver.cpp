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

#include "upnpmediaserver.h"

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
        QString url = upnpDevice()->device()->locations()[0].toString(QUrl::RemoveScheme);

        return (scheme + url);
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