/*  This file is part of the KDE project
    Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>

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

#ifndef SOLID_BACKENDS_FAKEHW_FAKEPROCESSOR_H
#define SOLID_BACKENDS_FAKEHW_FAKEPROCESSOR_H

#include "fakedeviceinterface.h"
#include <solid/ifaces/processor.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeProcessor : public FakeDeviceInterface, public Solid::Ifaces::Processor
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Processor)

public:
    explicit FakeProcessor(FakeDevice *device);
    ~FakeProcessor();

public Q_SLOTS:
    virtual int number() const;
    virtual int maxSpeed() const;
    virtual bool canChangeFrequency() const;
    virtual Solid::Processor::InstructionSets instructionSets() const;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEPROCESSOR_H
