/*
    Copyright 2007 Kevin Ottens <ervin@kde.org>

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

#include "fakestorageaccess.h"

using namespace Solid::Backends::Fake;

FakeStorageAccess::FakeStorageAccess(FakeDevice *device)
    : FakeDeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString, int>&)),
            this, SLOT(onPropertyChanged(const QMap<QString, int>&)));
}

FakeStorageAccess::~FakeStorageAccess()
{

}


bool FakeStorageAccess::isAccessible() const
{
    return fakeDevice()->property("isMounted").toBool();
}

QString FakeStorageAccess::filePath() const
{
    return fakeDevice()->property("mountPoint").toString();
}

bool FakeStorageAccess::isIgnored() const
{
    return fakeDevice()->property("isIgnored").toBool();
}

bool FakeStorageAccess::setup()
{
    if (fakeDevice()->isBroken() || isAccessible()) {
        return false;
    } else {
        fakeDevice()->setProperty("isMounted", true);
        return true;
    }
}

bool FakeStorageAccess::teardown()
{
    if (fakeDevice()->isBroken() || !isAccessible()) {
        return false;
    } else {
        fakeDevice()->setProperty("isMounted", false);
        return true;
    }
}

void Solid::Backends::Fake::FakeStorageAccess::onPropertyChanged(const QMap<QString,int> &changes)
{
    foreach (const QString &property, changes.keys()) {
        if (property=="isMounted") {
            emit accessibilityChanged(fakeDevice()->property("isMounted").toBool(), fakeDevice()->udi());
        }
    }
}

#include "backends/fakehw/fakestorageaccess.moc"
