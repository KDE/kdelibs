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

#include "winstorageaccess.h"
#include "winblock.h"
#include "winstoragedrive.h"


using namespace Solid::Backends::Win;

WinStorageAccess::WinStorageAccess(WinDevice *device) :
    WinInterface(device)
{
}

WinStorageAccess::~WinStorageAccess()
{
}

bool WinStorageAccess::isAccessible() const
{
    return true;
}

QString WinStorageAccess::filePath() const
{
    return WinBlock::driveLetterFromUdi(m_device->udi()).append("/");
}

bool WinStorageAccess::isIgnored() const
{
    return false;
}

bool WinStorageAccess::setup()
{
    return true;
}

bool WinStorageAccess::teardown()
{
    //only allow eject if we are an usb stick
    //else we get "The request could not be performed because of an I/O device error. 1117"
    if(WinStorageDrive(m_device).driveType() == Solid::StorageDrive::MemoryStick)
    {
        WinDeviceManager::deviceAction(WinBlock::driveLetterFromUdi(m_device->udi()),IOCTL_STORAGE_EJECT_MEDIA);
    }
    return true;
}


#include "winstorageaccess.moc"
