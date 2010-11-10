/*
    Copyright 2010 Rafael Fernández López <ereslibre@kde.org>

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

#include "udevprocessor.h"

#include "udevdevice.h"

using namespace Solid::Backends::UDev;

Processor::Processor(UDevDevice *device)
    : DeviceInterface(device)
{

}

Processor::~Processor()
{

}

int Processor::number() const
{
    QRegExp regExp(":(\\d+)$");
    if (regExp.indexIn(m_device->udi()) != -1) {
        return regExp.cap(1).toInt();
    }
    return -1;
}

int Processor::maxSpeed() const
{
    // TODO: source ?
    return 0;
}

bool Processor::canChangeFrequency() const
{
    // TODO: source ?
    return false;
}

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    // TODO: source ?
    return Solid::Processor::InstructionSets();
}

#include "backends/udev/udevprocessor.moc"
