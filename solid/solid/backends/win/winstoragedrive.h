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
#ifndef WINSTORAGEDRIVE_H
#define WINSTORAGEDRIVE_H

#include <solid/ifaces/storagedrive.h>


#include "windevice.h"
#include "winblock.h"

namespace Solid
{
namespace Backends
{
namespace Win
{

class WinStorageDrive : public WinBlock ,  public virtual Solid::Ifaces::StorageDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageDrive)
public:
    WinStorageDrive(WinDevice *device);
    ~WinStorageDrive();

    virtual Solid::StorageDrive::Bus bus() const;

    virtual Solid::StorageDrive::DriveType driveType() const;


    virtual bool isRemovable() const;


    virtual bool isHotpluggable() const;

    virtual qulonglong size() const;
};
}
}
}

#endif // WINSTORAGEDRIVE_H
