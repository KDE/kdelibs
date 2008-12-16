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

#ifndef SOLID_BACKENDS_FAKEHW_FAKEOPTICALDISC_H
#define SOLID_BACKENDS_FAKEHW_FAKEOPTICALDISC_H

#include "fakevolume.h"
#include <solid/ifaces/opticaldisc.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeOpticalDisc : public FakeVolume, virtual public Solid::Ifaces::OpticalDisc
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::OpticalDisc)

public:
    explicit FakeOpticalDisc(FakeDevice *device);
    virtual ~FakeOpticalDisc();

public Q_SLOTS:
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

#endif // SOLID_BACKENDS_FAKEHW_FAKEOPTICALDISC_H
