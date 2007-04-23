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

#include "camera.h"

Camera::Camera(HalDevice *device)
    : DeviceInterface(device)
{

}

Camera::~Camera()
{

}


Solid::Camera::AccessType Camera::accessMethod() const
{
    QString method = m_device->property("camera.access_method").toString();

    if (method=="storage")
    {
        return Solid::Camera::MassStorage;
    }
    else if (method=="ptp")
    {
        return Solid::Camera::Ptp;
    }
    else
    {
        return Solid::Camera::Proprietary;
    }
}

bool Camera::isGphotoSupported() const
{
    return m_device->property("camera.libgphoto2.support").toBool();
}

#include "camera.moc"
