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

#ifndef SOLID_BACKENDS_HAL_VOLUME_H
#define SOLID_BACKENDS_HAL_VOLUME_H

#include <solid/ifaces/storagevolume.h>
#include "halblock.h"

namespace Solid
{
namespace Backends
{
namespace Hal
{
class Volume : public Block, virtual public Solid::Ifaces::StorageVolume
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageVolume)

public:
    Volume(HalDevice *device);
    virtual ~Volume();

    virtual bool isIgnored() const;
    virtual Solid::StorageVolume::UsageType usage() const;
    virtual QString fsType() const;
    virtual QString label() const;
    virtual QString uuid() const;
    virtual qulonglong size() const;
    virtual QString encryptedContainerUdi() const;
};
}
}
}

#endif // SOLID_BACKENDS_HAL_VOLUME_H
