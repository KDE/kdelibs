/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>
    Copyright 2007 Will Stephenson <wstephenson@kde.org>

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

#include "wmivideo.h"

using namespace Solid::Backends::Wmi;

Video::Video(WmiDevice *device)
    : DeviceInterface(device)
{

}

Video::~Video()
{

}

QStringList Video::supportedProtocols() const
{
    QStringList protocols;
    protocols << QString( "video4linux" );
    return protocols;
}

QStringList Video::supportedDrivers(QString protocol) const
{
    // if there is a difference between v4l and v4l2
    QStringList drivers;
    drivers << QString( "video4linux" );
    return drivers;
}

QVariant Solid::Backends::Wmi::Video::driverHandle(const QString &driver) const
{
    if (driver=="video4linux") {
        return m_device->property("video4linux.device");
    }
    // TODO: Fill in the blank for other drivers

    return QVariant();
}

#include "backends/wmi/wmivideo.moc"
