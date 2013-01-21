/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

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
#include "winstoragedrive.h"


using namespace Solid::Backends::Win;

WinStorageDrive::WinStorageDrive(WinDevice *device):
    WinBlock(device)
{
}

WinStorageDrive::~WinStorageDrive()
{
}

Solid::StorageDrive::Bus WinStorageDrive::bus() const
{
    return Solid::StorageDrive::Ide;
}

Solid::StorageDrive::DriveType WinStorageDrive::driveType() const
{
    return Solid::StorageDrive::HardDisk;
}

bool WinStorageDrive::isRemovable() const
{
    return false;
}

bool WinStorageDrive::isHotpluggable() const
{
    return false;
}

qulonglong WinStorageDrive::size() const
{
    return 0;
}

#include "winstoragedrive.moc"
