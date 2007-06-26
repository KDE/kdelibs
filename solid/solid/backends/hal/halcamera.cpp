/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "halcamera.h"

Camera::Camera(HalDevice *device)
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

#include "backends/hal/halcamera.moc"
