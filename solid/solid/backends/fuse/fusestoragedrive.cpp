/*
 *   Copyright (C) 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "fusestoragedrive.h"
#include <QDebug>

namespace Solid
{
namespace Backends
{
namespace Fuse
{

FuseStorageDrive::FuseStorageDrive(FuseDevice *device)
    : QObject(device)
{
    qDebug() << "FuseStorageDrive constructor" << device->device();
}

FuseStorageDrive::~FuseStorageDrive()
{
}

bool FuseStorageDrive::isHotpluggable() const
{
    qDebug() << "isHotpluggable true";
    return true;
}

bool FuseStorageDrive::isRemovable() const
{
    qDebug() << "isRemovable true";
    return true;
}

qulonglong FuseStorageDrive::size() const
{
    // TODO: make this real
    return 1024;
}

Solid::StorageDrive::Bus FuseStorageDrive::bus() const
{
    return Solid::StorageDrive::Platform;
}

Solid::StorageDrive::DriveType FuseStorageDrive::driveType() const
{
    return Solid::StorageDrive::HardDisk;
}

}
}
}
