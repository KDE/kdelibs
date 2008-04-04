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

#ifndef SOLID_BACKENDS_WMI_BLOCK_H
#define SOLID_BACKENDS_WMI_BLOCK_H

#include <solid/ifaces/block.h>
#include "wmideviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace Wmi
{
class Block : public DeviceInterface, virtual public Solid::Ifaces::Block
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Block)

public:
    Block(WmiDevice *device);
    virtual ~Block();

    virtual int deviceMajor() const;
    virtual int deviceMinor() const;
    virtual QString device() const;
};
}
}
}

#endif // SOLID_BACKENDS_WMI_BLOCK_H
