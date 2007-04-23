/*  This file is part of the KDE project
    Copyright (C) 2005,2006 Kevin Ottens <ervin@kde.org>

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

#include "halbasictest.h"

#include <qtest_kde.h>

#include "halmanager.h"
#include <solid/ifaces/device.h>
#include <solid/ifaces/deviceinterface.h>
#include <solid/ifaces/processor.h>
#include "processor.h"

QTEST_KDEMAIN_CORE(HalBasicTest)


void HalBasicTest::testBasic()
{
    HalManager *manager = new HalManager(0, QStringList());

    QVERIFY(manager->deviceExists("/org/freedesktop/Hal/devices/computer"));
    QVERIFY(!manager->allDevices().isEmpty());

    QVERIFY(!manager->devicesFromQuery(QString(), Solid::DeviceInterface::Processor).isEmpty());

    QString proc_udi = manager->devicesFromQuery(QString(), Solid::DeviceInterface::Processor).at(0);

    HalDevice *processor = qobject_cast<HalDevice *>(manager->createDevice(proc_udi));

    QCOMPARE(processor->udi(), proc_udi);
    QCOMPARE(processor->parentUdi(), QString("/org/freedesktop/Hal/devices/computer"));
    QVERIFY(!processor->allProperties().isEmpty());
    QVERIFY(processor->propertyExists("info.product"));
    QVERIFY(!processor->propertyExists("the.meaning.of.life"));
    QVERIFY(processor->queryDeviceInterface(Solid::DeviceInterface::Processor));
    QVERIFY(!processor->queryDeviceInterface(Solid::DeviceInterface::OpticalDisc));

    QObject *interface = processor->createDeviceInterface(Solid::DeviceInterface::Processor);
    Solid::Ifaces::Processor *proc_iface = qobject_cast<Solid::Ifaces::Processor *>(interface);

    QVERIFY(proc_iface!=0);

    QVERIFY(!processor->isLocked());
    QVERIFY(processor->lock("No reason..."));
    QVERIFY(processor->isLocked());
    QCOMPARE(processor->lockReason(), QString("No reason..."));
    QVERIFY(!processor->lock("Need a reason?"));
    QVERIFY(processor->unlock());

    QCOMPARE(interface->parent(), processor);

    delete processor;
    delete manager;
}

void HalBasicTest::testSignalHandling()
{
    HalManager *manager = new HalManager(0, QStringList());
    m_device = qobject_cast<HalDevice *>(manager->createDevice("/org/freedesktop/Hal/devices/computer"));

    connect(m_device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));

    QVERIFY(!m_device->isLocked());
    m_signalProcessed = false;
    m_device->lock("Still no reason... really");
    QVERIFY(m_device->isLocked());
    QVERIFY(m_signalProcessed);

    delete m_device;
    delete manager;
}

void HalBasicTest::slotPropertyChanged(const QMap<QString,int> &changes)
{
    QVERIFY(m_device->isLocked());
    m_signalProcessed = true;
}

#include "halbasictest.moc"

