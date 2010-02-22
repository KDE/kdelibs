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
#include "fakehardwaretest.h"

// Qt includes
#include <QtTest/QtTest>
#include <QtCore/QStringList>

// Solid includes
#include <solid/ifaces/device.h>
#include <solid/ifaces/deviceinterface.h>
#include <solid/ifaces/processor.h>

// Local includes
#include "solid/backends/fakehw/fakemanager.h"
#include "solid/backends/fakehw/fakedevice.h"

QTEST_MAIN(FakeHardwareTest)

void FakeHardwareTest::testFakeBackend()
{
    Solid::Backends::Fake::FakeManager *fakeManager = new Solid::Backends::Fake::FakeManager(0, TEST_DATA);

    QVERIFY(!fakeManager->allDevices().isEmpty());
    QObject *computer = fakeManager->createDevice("/org/kde/solid/fakehw/computer");
    QVERIFY(computer!=0);
    QVERIFY(fakeManager->createDevice("/com/helloworld/troll/compiutor")==0);

    Solid::Backends::Fake::FakeDevice *device = static_cast<Solid::Backends::Fake::FakeDevice *>(fakeManager->createDevice("/org/kde/solid/fakehw/acpi_CPU0"));

    QCOMPARE(device->udi(), QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(device->parentUdi(), QString("/org/kde/solid/fakehw/computer"));
    QCOMPARE(device->vendor(), QString("Acme Corporation"));
    QCOMPARE(device->product(), QString("Solid Processor #0"));

    QCOMPARE(device->property("number").toString(), QString("0"));
    QVERIFY(device->propertyExists("number"));
    QVERIFY(!device->propertyExists("youstfuqewerrernoob"));

    QVERIFY(device->queryDeviceInterface(Solid::DeviceInterface::Processor));

    QObject *interface = device->createDeviceInterface(Solid::DeviceInterface::Processor);
    Solid::Ifaces::Processor *processor = qobject_cast<Solid::Ifaces::Processor *>(interface);

    QCOMPARE(processor->number(), 0);
    QCOMPARE(processor->canChangeFrequency(), true);
    QCOMPARE((int)processor->maxSpeed(), 3200);

    Solid::Processor::InstructionSets instructionsets;
    instructionsets |= Solid::Processor::IntelMmx;
    instructionsets |= Solid::Processor::IntelSse;
    QCOMPARE(processor->instructionSets(), instructionsets);

    delete processor;
    delete device;
    delete computer;
    delete fakeManager;
}

#include "fakehardwaretest.moc"
