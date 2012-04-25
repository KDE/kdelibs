/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010 Lukas Tinkl <ltinkl@redhat.com>
    Copyright 2012 Ivan Cukic <ivan.cukic@kde.org>

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

#ifndef FUSESTORAGEDRIVE_H
#define FUSESTORAGEDRIVE_H

#include <ifaces/storagedrive.h>

#include "fusedevice.h"

namespace Solid
{
namespace Backends
{
namespace Fuse
{

class FuseStorageDrive: public QObject, virtual public Solid::Ifaces::StorageDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageDrive)
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)

public:
    FuseStorageDrive(FuseDevice *device);
    virtual ~FuseStorageDrive();

    virtual qulonglong size() const;
    virtual bool isHotpluggable() const;
    virtual bool isRemovable() const;
    virtual Solid::StorageDrive::DriveType driveType() const;
    virtual Solid::StorageDrive::Bus bus() const;

    virtual QString device() const { return QString::fromLatin1("/dev/fuse"); }
    virtual int deviceMajor() const { return 0; };
    virtual int deviceMinor() const { return 0; };


};

}
}
}

#endif // FUSESTORAGEDRIVE_H
