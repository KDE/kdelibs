/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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
