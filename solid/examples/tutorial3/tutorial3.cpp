/*  This file is part of the KDE project
    Copyright (C) 2007 Christopher Blauvelt <cblauvelt@gmail.com>

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

//QT specific includes
#include <QList>

//solid specific includes
#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/processor.h>

//kde specific includes
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>

#include <iostream>

using namespace std;

int main(int args, char **argv)
{
    KComponentData componentData("tutorial3");

    //get a Processor
    QList<Solid::Device> list = Solid::Device::listFromType(Solid::DeviceInterface::Processor, QString());

    //take the first processor
    Solid::Device device = list[0];
    if(device.is<Solid::Processor>()) kDebug() << "We've got a processor!" << list.count() << "to be exact...";
    else kDebug() << "Device is not a processor.";

    Solid::Processor *processor = device.as<Solid::Processor>();
    kDebug() << "This processors maximum speed is: " << processor->maxSpeed();

    Solid::Processor::InstructionSets extensions = processor->instructionSets();
    kDebug() << "Intel MMX supported:" << (bool)(extensions & Solid::Processor::IntelMmx);
    kDebug() << "Intel SSE supported:" << (bool)(extensions & Solid::Processor::IntelSse);
    kDebug() << "Intel SSE2 supported:" << (bool)(extensions & Solid::Processor::IntelSse2);
    kDebug() << "Intel SSE3 supported:" << (bool)(extensions & Solid::Processor::IntelSse3);
    kDebug() << "Intel SSE4 supported:" << (bool)(extensions & Solid::Processor::IntelSse4);
    kDebug() << "AMD 3DNOW supported:" << (bool)(extensions & Solid::Processor::Amd3DNow);
    kDebug() << "PPC AltiVec supported:" << (bool)(extensions & Solid::Processor::AltiVec);

    return 0;
}

#include "tutorial3.moc"
