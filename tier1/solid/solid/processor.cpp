/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "processor.h"
#include "processor_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/processor.h>


Solid::Processor::Processor(QObject *backendObject)
    : DeviceInterface(*new ProcessorPrivate(), backendObject)
{
}

Solid::Processor::~Processor()
{

}

int Solid::Processor::number() const
{
    Q_D(const Processor);
    return_SOLID_CALL(Ifaces::Processor *, d->backendObject(), 0, number());
}

int Solid::Processor::maxSpeed() const
{
    Q_D(const Processor);
    return_SOLID_CALL(Ifaces::Processor *, d->backendObject(), 0, maxSpeed());
}

bool Solid::Processor::canChangeFrequency() const
{
    Q_D(const Processor);
    return_SOLID_CALL(Ifaces::Processor *, d->backendObject(), false, canChangeFrequency());
}

Solid::Processor::InstructionSets Solid::Processor::instructionSets() const
{
    Q_D(const Processor);
    return_SOLID_CALL(Ifaces::Processor *, d->backendObject(), InstructionSets(), instructionSets());
}

#include "processor.moc"
