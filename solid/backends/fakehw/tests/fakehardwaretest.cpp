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

// KDE-QTestLib include
#include <qtest_kde.h>

// Qt includes
#include <QStringList>

// Solid includes
#include <solid/ifaces/device.h>
#include <solid/ifaces/capability.h>
#include <solid/ifaces/processor.h>

// Local includes
#include "fakemanager.h"
#include "fakedevice.h"

QTEST_KDEMAIN( FakeHardwareTest, NoGUI )

void FakeHardwareTest::testFakeBackend()
{
    FakeManager *fakeManager = new FakeManager(0, QStringList(), TEST_DATA );

    QVERIFY( !fakeManager->allDevices().isEmpty() );
    QVERIFY( fakeManager->deviceExists("/org/kde/solid/fakehw/computer") );
    QVERIFY( !fakeManager->deviceExists("/com/helloworld/troll/compiutor") );

    FakeDevice *device = static_cast<FakeDevice*>(fakeManager->createDevice("/org/kde/solid/fakehw/acpi_CPU0"));

    QCOMPARE( device->udi(), QString("/org/kde/solid/fakehw/acpi_CPU0") );
    QCOMPARE( device->parentUdi(), QString("/org/kde/solid/fakehw/computer") );
    QCOMPARE( device->vendor(), QString("Acme Corporation") );
    QCOMPARE( device->product(), QString("Solid Processor #0") );

    QCOMPARE( device->property("number").toString(), QString("0") );
    QVERIFY( device->propertyExists("number") );
    QVERIFY( !device->propertyExists("youstfuqewerrernoob") );

    QVERIFY( device->queryCapability(Capability::Processor) );

    QObject *capability = device->createCapability( Solid::Ifaces::Capability::Processor );
    Solid::Ifaces::Processor *processor = qobject_cast<Solid::Ifaces::Processor*>( capability );

    QCOMPARE( processor->number(), 0 );
    QCOMPARE( processor->canThrottle(), true );
    QCOMPARE( (int)processor->maxSpeed(), 3200 );
}

#include "fakehardwaretest.moc"
