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
