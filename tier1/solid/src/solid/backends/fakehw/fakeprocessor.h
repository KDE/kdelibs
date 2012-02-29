/*
    Copyright 2006 Michaël Larouche <michael.larouche@kdemail.net>

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
