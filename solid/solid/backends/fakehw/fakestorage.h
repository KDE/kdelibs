/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davide.bettio@kdemail.net>

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

#ifndef SOLID_BACKENDS_FAKEHW_FAKESTORAGE_H
#define SOLID_BACKENDS_FAKEHW_FAKESTORAGE_H

#include "fakeblock.h"
#include <solid/ifaces/storagedrive.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeStorage : public FakeBlock, virtual public Solid::Ifaces::StorageDrive
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageDrive)

public:
    explicit FakeStorage(FakeDevice *device);
    ~FakeStorage();

public Q_SLOTS:
    virtual Solid::StorageDrive::Bus bus() const;
    virtual Solid::StorageDrive::DriveType driveType() const;

    virtual bool isRemovable() const;
    virtual bool isHotpluggable() const;
    virtual qulonglong size() const;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKESTORAGE_H
