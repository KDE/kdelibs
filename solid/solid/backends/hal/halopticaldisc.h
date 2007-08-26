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

#ifndef SOLID_BACKENDS_HAL_OPTICALDISC_H
#define SOLID_BACKENDS_HAL_OPTICALDISC_H

#include <solid/ifaces/opticaldisc.h>
#include "halvolume.h"

namespace Solid
{
namespace Backends
{
namespace Hal
{
class OpticalDisc : public Volume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    OpticalDisc(HalDevice *device);
    virtual ~OpticalDisc();

    virtual Solid::OpticalDisc::ContentTypes availableContent() const;
    virtual Solid::OpticalDisc::DiscType discType() const;
    virtual bool isAppendable() const;
    virtual bool isBlank() const;
    virtual bool isRewritable() const;
    virtual qulonglong capacity() const;
};
}
}
}

#endif // SOLID_BACKENDS_HAL_OPTICALDISC_H
