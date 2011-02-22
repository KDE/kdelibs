/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#include "halprocessor.h"

#include "haldevice.h"
#include "../shared/cpufeatures.h"

using namespace Solid::Backends::Hal;

Processor::Processor(HalDevice *device)
    : DeviceInterface(device)
{

}

Processor::~Processor()
{

}

int Processor::number() const
{
    return m_device->prop("processor.number").toInt();
}

int Processor::maxSpeed() const
{
    // the property is not mandatory in HAL
    return m_device->prop("processor.maximum_speed").toInt();
}

bool Processor::canChangeFrequency() const
{
    return m_device->prop("processor.can_throttle").toBool();
}

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    static Solid::Processor::InstructionSets cpuextensions = Solid::Backends::Shared::cpuFeatures();

    return cpuextensions;
}

#include "backends/hal/halprocessor.moc"
