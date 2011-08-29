/*
    Copyright 2010 Alex Fiestas <alex@eyeos.org>
    Copyright 2010 Rafael Fernández López <ereslibre@kde.org>
    Copyright 2010 UFO Coders <info@ufocoders.com>

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

#include "udevvideo.h"

using namespace Solid::Backends::UDev;

Video::Video(UDevDevice *device)
    : DeviceInterface(device)
{

}

Video::~Video()
{

}

QStringList Video::supportedProtocols() const
{
    QStringList protocols;
    protocols << QLatin1String("video4linux");
    return protocols;
}

QStringList Video::supportedDrivers(QString protocol) const
{
    QStringList drivers;

    if (m_device->propertyExists("ID_USB_DRIVER"))
        drivers << m_device->property("ID_USB_DRIVER").toString();

    if (protocol == QLatin1String("video4linux")) {
        drivers << QLatin1String("video4linux"); //Retrocompatibility with KDE < 4.3
        if (m_device->property("ID_V4L_VERSION").toInt() == 2) {
            drivers << QLatin1String("video4linux2");
        } else {
            drivers << QLatin1String("video4linux1");
        }
    }
    return drivers;
}

QVariant Video::driverHandle(const QString &driver) const
{
    if (driver == QLatin1String("video4linux") || driver == QLatin1String("video4linux1") || driver == QLatin1String("video4linux2")) {
        return m_device->property("DEVNAME");  // physical device, eg. /dev/video0
    }

    return QVariant();
}

#include "backends/udev/udevvideo.moc"
