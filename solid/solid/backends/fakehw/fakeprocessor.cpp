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

#include "fakeprocessor.h"

#include <QtCore/QStringList>

using namespace Solid::Backends::Fake;

FakeProcessor::FakeProcessor(FakeDevice *device)
 : FakeDeviceInterface(device)
{
}

FakeProcessor::~FakeProcessor()
{
}

int FakeProcessor::number() const
{
    return fakeDevice()->property("number").toInt();
}

int FakeProcessor::maxSpeed() const
{
    return fakeDevice()->property("maxSpeed").toInt();
}

bool FakeProcessor::canChangeFrequency() const
{
    return fakeDevice()->property("canChangeFrequency").toBool();
}

Solid::Processor::InstructionSets FakeProcessor::instructionSets() const
{
    Solid::Processor::InstructionSets result;

    QString str = fakeDevice()->property("instructionSets").toString();

    QStringList extension_list = str.split(',');

    foreach (const QString &extension_str, extension_list) {
        if (extension_str == "mmx") {
            result |= Solid::Processor::IntelMmx;
        }
        else if (extension_str == "sse") {
            result |= Solid::Processor::IntelSse;
        }
        else if (extension_str == "sse2") {
            result |= Solid::Processor::IntelSse2;
        }
        else if (extension_str == "sse3") {
            result |= Solid::Processor::IntelSse3;
        }
        else if (extension_str == "sse4") {
            result |= Solid::Processor::IntelSse4;
        }
        else if (extension_str == "3dnow") {
            result |= Solid::Processor::Amd3DNow;
        }
        else if (extension_str == "altivec") {
            result |= Solid::Processor::AltiVec;
        }
     }

    return result;

}

#include "backends/fakehw/fakeprocessor.moc"
