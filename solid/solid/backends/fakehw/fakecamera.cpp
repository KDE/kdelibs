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

#include "fakecamera.h"

using namespace Solid::Backends::Fake;

FakeCamera::FakeCamera(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakeCamera::~FakeCamera()
{

}

QStringList FakeCamera::supportedProtocols() const
{
    QStringList res;
    QString method = fakeDevice()->property("accessMethod").toString();

    res << method;

    return res;
}

QStringList FakeCamera::supportedDrivers(QString /*protocol*/) const
{
    QStringList res;

    if (fakeDevice()->property("gphotoSupport").toBool()) {
        res << "gphoto";
    }

    return res;
}

QVariant Solid::Backends::Fake::FakeCamera::driverHandle(const QString &driver) const
{
    Q_UNUSED(driver);
    return QVariant();
}

#include "backends/fakehw/fakecamera.moc"
