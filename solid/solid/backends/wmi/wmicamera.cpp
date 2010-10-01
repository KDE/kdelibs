/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#include "wmicamera.h"

using namespace Solid::Backends::Wmi;

Camera::Camera(WmiDevice *device)
    : DeviceInterface(device)
{

}

Camera::~Camera()
{

}

QStringList Camera::supportedProtocols() const
{
    QStringList protocols;

    QString method = m_device->property("camera.access_method").toString();

    protocols << method;

    return protocols;
}

QStringList Camera::supportedDrivers(QString /*protocol*/) const
{
    QStringList res;

    if (m_device->property("camera.libgphoto2.support").toBool()) {
        res << "gphoto";
    }

    return res;
}

QVariant Solid::Backends::Wmi::Camera::driverHandle(const QString &driver) const
{
    if (driver=="gphoto"
     && m_device->property("info.subsystem").toString()=="usb") {
        QVariantList list;

        list << "usb"
             << m_device->property("usb.vendor_id")
             << m_device->property("usb.product_id");

        return list;
    }

    return QVariant();
}

#include "backends/wmi/wmicamera.moc"
