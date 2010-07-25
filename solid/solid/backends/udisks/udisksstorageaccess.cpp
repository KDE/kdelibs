/*  Copyright 2010  Michael Zanetti <mzanetti@kde.org>

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
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "udisksstorageaccess.h"

using namespace Solid::Backends::UDisks;

UDisksStorageAccess::UDisksStorageAccess(UDisksDevice* device)
    : DeviceInterface(device)
{

}

UDisksStorageAccess::~UDisksStorageAccess()
{
  
}

void UDisksStorageAccess::teardownRequested(const QString& udi)
{

}

void UDisksStorageAccess::setupRequested(const QString& udi)
{

}

void UDisksStorageAccess::teardownDone(Solid::ErrorType error, QVariant resultData, const QString& udi)
{

}

void UDisksStorageAccess::setupDone(Solid::ErrorType error, QVariant resultData, const QString& udi)
{

}

void UDisksStorageAccess::accessibilityChanged(bool accessible, const QString& udi)
{

}

bool UDisksStorageAccess::teardown()
{
    return false;
}

bool UDisksStorageAccess::setup()
{
    return false;
}

QString UDisksStorageAccess::filePath() const
{
    return QString();
}

bool UDisksStorageAccess::isAccessible() const
{
    return false;
}

