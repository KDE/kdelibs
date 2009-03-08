/*  This file is part of the KDE project
    Copyright (C) 2009 Harald Fernengel <harry@kdevelop.org>

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

#include "iokitprocessor.h"
#include "iokitdevice.h"

#include <QtCore/qdebug.h>

using namespace Solid::Backends::IOKit;

Processor::Processor(IOKitDevice *device)
    : DeviceInterface(device)
{
    //IOKitDevice parent(device->parentUdi());
}

Processor::~Processor()
{

}

int Processor::number() const
{
    return m_device->property(QLatin1String("IOCPUNumber")).toInt();
}

int Processor::maxSpeed() const
{
    return 0; // TODO
}

bool Processor::canChangeFrequency() const
{
    return false; // TODO
}

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    return 0; // TODO
}

#include "backends/iokit/iokitprocessor.moc"
