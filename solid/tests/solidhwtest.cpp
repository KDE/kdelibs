/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#include "solidhwtest.h"

#include <QtTest/QtTest>

#include <solid/devicenotifier.h>
#include <solid/device.h>
#include <solid/genericinterface.h>
#include <solid/processor.h>
#include <solid/storageaccess.h>
#include <solid/storagevolume.h>
#include <solid/predicate.h>
#include "solid/managerbase_p.h"

#include <fakemanager.h>
#include <fakedevice.h>

#include <stdlib.h>

#ifndef FAKE_COMPUTER_XML
    #error "FAKE_COMPUTER_XML not set. An XML file describing a computer is required for this test"
#endif

QTEST_MAIN(SolidHwTest)

void SolidHwTest::initTestCase()
{
    setenv("SOLID_FAKEHW", FAKE_COMPUTER_XML, 1);
    Solid::ManagerBasePrivate *manager
        = dynamic_cast<Solid::ManagerBasePrivate*>(Solid::DeviceNotifier::instance());
    fakeManager = qobject_cast<Solid::Backends::Fake::FakeManager*>(manager->managerBackends().first());
}

void SolidHwTest::testAllDevices()
{
    QList<Solid::Device> devices = Solid::Device::allDevices();

    // Verify that the framework reported correctly the devices available
    // in the backend.
    QSet<QString> expected_udis, received_udis;

    expected_udis = QSet<QString>::fromList(fakeManager->allDevices());

    foreach (const Solid::Device &dev , devices)
    {
        received_udis << dev.udi();
    }

    QCOMPARE(expected_udis, received_udis);
}

void SolidHwTest::testDeviceBasicFeatures()
{
    // Retrieve a valid Device object
    Solid::Device valid_dev("/org/kde/solid/fakehw/storage_model_solid_writer");

    QCOMPARE(valid_dev.isValid(), true);


    // A few attempts at creating invalid Device objects
    Solid::Device invalid_dev("uhoh? doesn't exist, I guess");
    QCOMPARE(invalid_dev.isValid(), false);
    invalid_dev = Solid::Device(QString());
    QCOMPARE(invalid_dev.isValid(), false);
    invalid_dev = Solid::Device();
    QCOMPARE(invalid_dev.isValid(), false);



    QCOMPARE(valid_dev.udi(), QString("/org/kde/solid/fakehw/storage_model_solid_writer"));
    QCOMPARE(invalid_dev.udi(), QString());


    // Query properties
    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->propertyExists("name"), true);
    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->propertyExists("foo.bar"), false);
    QCOMPARE((QObject *)invalid_dev.as<Solid::GenericInterface>(), (QObject *)0);

    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->property("name"), QVariant("Solid IDE DVD Writer"));
    QVERIFY(!valid_dev.as<Solid::GenericInterface>()->property("foo.bar").isValid());

    Solid::Backends::Fake::FakeDevice *fake_device = fakeManager->findDevice("/org/kde/solid/fakehw/storage_model_solid_writer");
    QMap<QString, QVariant> expected_properties = fake_device->allProperties();

    QCOMPARE(valid_dev.as<Solid::GenericInterface>()->allProperties(), expected_properties);


    // Query device interfaces
    QCOMPARE(valid_dev.isDeviceInterface(Solid::DeviceInterface::StorageDrive), true);
    QCOMPARE(valid_dev.isDeviceInterface(Solid::DeviceInterface::OpticalDrive), true);
    QCOMPARE(valid_dev.isDeviceInterface(Solid::DeviceInterface::StorageVolume), false);

    QCOMPARE(invalid_dev.isDeviceInterface(Solid::DeviceInterface::Unknown), false);
    QCOMPARE(invalid_dev.isDeviceInterface(Solid::DeviceInterface::StorageDrive), false);


    // Query parent
    QCOMPARE(valid_dev.parentUdi(), QString("/org/kde/solid/fakehw/pci_002_ide_1_0"));
    QCOMPARE(valid_dev.parent().udi(), Solid::Device("/org/kde/solid/fakehw/pci_002_ide_1_0").udi());

    QVERIFY(!invalid_dev.parent().isValid());
    QVERIFY(invalid_dev.parentUdi().isEmpty());


    // Query vendor/product
    QCOMPARE(valid_dev.vendor(), QString("Acme Corporation"));
    QCOMPARE(valid_dev.product(), QString("Solid IDE DVD Writer"));

    QCOMPARE(invalid_dev.vendor(), QString());
    QCOMPARE(invalid_dev.product(), QString());
}

void SolidHwTest::testManagerSignals()
{
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");

    // Heh, we missed a processor in this system ;-)
    // We're going to add this device, and check that the signal has been
    // properly emitted by the manager
    QSignalSpy added(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)));
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(added.count(), 1);
    QCOMPARE(added.at(0).at(0).toString(), QString("/org/kde/solid/fakehw/acpi_CPU0"));

    // Moreover we check that the device is really available
    Solid::Device cpu("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(cpu.isValid());


    // Finally we remove the device and spy the corresponding signal again
    QSignalSpy removed(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)));
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(added.count(), 1);
    QCOMPARE(added.at(0).at(0).toString(), QString("/org/kde/solid/fakehw/acpi_CPU0"));

    // The Device object should become automatically invalid
    QVERIFY(!cpu.isValid());

    // Restore original state
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");
}

void SolidHwTest::testDeviceSignals()
{
    // A button is a nice device for testing state changes, isn't it?
    Solid::Backends::Fake::FakeDevice *fake = fakeManager->findDevice("/org/kde/solid/fakehw/acpi_LID0");
    Solid::Device device("/org/kde/solid/fakehw/acpi_LID0");

    // We'll spy our button
    connect(device.as<Solid::GenericInterface>(), SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));
    QSignalSpy condition_raised(device.as<Solid::GenericInterface>(), SIGNAL(conditionRaised(QString, QString)));

    fake->setProperty("stateValue", true); // The button is now pressed (modified property)
    fake->raiseCondition("Lid Closed", "Why not?"); // Since it's a LID we notify this change
    fake->setProperty("hactar", 42); // We add a property
    fake->removeProperty("hactar"); // We remove a property

    // 3 property changes occurred in the device
    QCOMPARE(m_changesList.count(), 3);

    QMap<QString,int> changes;

    // First one is a "PropertyModified" for "button.state"
    changes = m_changesList.at(0);
    QCOMPARE(changes.count(), 1);
    QVERIFY(changes.contains("stateValue"));
    QCOMPARE(changes["stateValue"], (int)Solid::GenericInterface::PropertyModified);

    // Second one is a "PropertyAdded" for "hactar"
    changes = m_changesList.at(1);
    QCOMPARE(changes.count(), 1);
    QVERIFY(changes.contains("hactar"));
    QCOMPARE(changes["hactar"], (int)Solid::GenericInterface::PropertyAdded);

    // Third one is a "PropertyRemoved" for "hactar"
    changes = m_changesList.at(2);
    QCOMPARE(changes.count(), 1);
    QVERIFY(changes.contains("hactar"));
    QCOMPARE(changes["hactar"], (int)Solid::GenericInterface::PropertyRemoved);



    // Only one condition has been raised in the device
    QCOMPARE(condition_raised.count(), 1);

    // It must be identical to the condition we raised by hand
    QCOMPARE(condition_raised.at(0).at(0).toString(), QString("Lid Closed"));
    QCOMPARE(condition_raised.at(0).at(1).toString(), QString("Why not?"));
}

void SolidHwTest::testDeviceExistence()
{
    QCOMPARE(Solid::Device("/org/kde/solid/fakehw/acpi_LID0").isValid(), true);
    QCOMPARE(Solid::Device("/org/kde/solid/fakehw/volume_label_SOLIDMAN_BEGINS").isValid(), true);

    // Note the extra space
    QCOMPARE(Solid::Device("/org/kde/solid/fakehw/computer ").isValid(), false);
    QCOMPARE(Solid::Device("#'({(à]").isValid(), false);
    QCOMPARE(Solid::Device(QString()).isValid(), false);

    // Now try to see if isValid() changes on plug/unplug events
    Solid::Device cpu("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(cpu.isValid());
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(!cpu.isValid());
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(cpu.isValid());
}

void SolidHwTest::testDeviceInterfaces()
{
    Solid::Device cpu("/org/kde/solid/fakehw/acpi_CPU0");

    Solid::DeviceInterface *iface = cpu.asDeviceInterface(Solid::DeviceInterface::Processor);
    Solid::DeviceInterface *processor = cpu.as<Solid::Processor>();

    QVERIFY(cpu.isDeviceInterface(Solid::DeviceInterface::Processor));
    QVERIFY(iface!=0);
    QCOMPARE(iface, processor);

    Solid::Device cpu2("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(cpu.as<Solid::Processor>(), cpu2.as<Solid::Processor>());
    QCOMPARE(cpu.as<Solid::GenericInterface>(), cpu2.as<Solid::GenericInterface>());

    QPointer<Solid::Processor> p = cpu.as<Solid::Processor>();
    QVERIFY(p!=0);
    fakeManager->unplug("/org/kde/solid/fakehw/acpi_CPU0");
    QVERIFY(p==0);
    fakeManager->plug("/org/kde/solid/fakehw/acpi_CPU0");

    QPointer<Solid::StorageVolume> v;
    QPointer<Solid::StorageVolume> v2;
    {
        Solid::Device partition("/org/kde/solid/fakehw/volume_uuid_f00ba7");
        v = partition.as<Solid::StorageVolume>();
        QVERIFY(v!=0);
        {
            Solid::Device partition2("/org/kde/solid/fakehw/volume_uuid_f00ba7");
            v2 = partition2.as<Solid::StorageVolume>();
            QVERIFY(v2!=0);
            QVERIFY(v==v2);
        }
        QVERIFY(v!=0);
        QVERIFY(v2!=0);
    }
    QVERIFY(v!=0);
    QVERIFY(v2!=0);
    fakeManager->unplug("/org/kde/solid/fakehw/volume_uuid_f00ba7");
    QVERIFY(v==0);
    QVERIFY(v2==0);
    fakeManager->plug("/org/kde/solid/fakehw/volume_uuid_f00ba7");
}

void SolidHwTest::testDeviceInterfaceIntrospection_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("value");

    QTest::newRow("DeviceInterface: Unknown") << "Unknown" << (int)Solid::DeviceInterface::Unknown;
    QTest::newRow("DeviceInterface: Processor") << "Processor" << (int)Solid::DeviceInterface::Processor;
    QTest::newRow("DeviceInterface: Block") << "Block" << (int)Solid::DeviceInterface::Block;
    QTest::newRow("DeviceInterface: StorageDrive") << "StorageDrive" << (int)Solid::DeviceInterface::StorageDrive;
    QTest::newRow("DeviceInterface: OpticalDrive") << "OpticalDrive" << (int)Solid::DeviceInterface::OpticalDrive;
    QTest::newRow("DeviceInterface: StorageVolume") << "StorageVolume" << (int)Solid::DeviceInterface::StorageVolume;
    QTest::newRow("DeviceInterface: OpticalDisc") << "OpticalDisc" << (int)Solid::DeviceInterface::OpticalDisc;
    QTest::newRow("DeviceInterface: Camera") << "Camera" << (int)Solid::DeviceInterface::Camera;
    QTest::newRow("DeviceInterface: PortableMediaPlayer") << "PortableMediaPlayer" << (int)Solid::DeviceInterface::PortableMediaPlayer;
    QTest::newRow("DeviceInterface: NetworkInterface") << "NetworkInterface" << (int)Solid::DeviceInterface::NetworkInterface;
    QTest::newRow("DeviceInterface: AcAdapter") << "AcAdapter" << (int)Solid::DeviceInterface::AcAdapter;
    QTest::newRow("DeviceInterface: Battery") << "Battery" << (int)Solid::DeviceInterface::Battery;
    QTest::newRow("DeviceInterface: Button") << "Button" << (int)Solid::DeviceInterface::Button;
    QTest::newRow("DeviceInterface: AudioInterface") << "AudioInterface" << (int)Solid::DeviceInterface::AudioInterface;
}

void SolidHwTest::testDeviceInterfaceIntrospection()
{
    QFETCH(QString, name);
    QFETCH(int, value);

    QCOMPARE(Solid::DeviceInterface::typeToString((Solid::DeviceInterface::Type)value), name);
    QCOMPARE((int)Solid::DeviceInterface::stringToType(name), value);
}

void SolidHwTest::testDeviceInterfaceIntrospectionCornerCases()
{
    QCOMPARE(Solid::DeviceInterface::typeToString((Solid::DeviceInterface::Type)-1), QString());
    QCOMPARE((int)Solid::DeviceInterface::stringToType("blup"), -1);
}

static QSet<QString> to_string_set(const QList<Solid::Device> &list)
{
    QSet<QString> res;

    foreach (const Solid::Device &device, list) {
        res << device.udi();
    }

    return res;
}

void SolidHwTest::testPredicate()
{
    Solid::Device dev("/org/kde/solid/fakehw/acpi_CPU0");

    Solid::Predicate p1 = Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3200)
                         & Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", true);
    Solid::Predicate p2 = Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3200)
                         & Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", false);
    Solid::Predicate p3 = Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3201)
                        | Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", true);
    Solid::Predicate p4 = Solid::Predicate(Solid::DeviceInterface::Processor, "maxSpeed", 3201)
                        | Solid::Predicate(Solid::DeviceInterface::Processor, "canChangeFrequency", false);
    Solid::Predicate p5 = Solid::Predicate::fromString("[[Processor.maxSpeed == 3201 AND Processor.canChangeFrequency == false] OR StorageVolume.mountPoint == '/media/blup']");

    QVERIFY(p1.matches(dev));
    QVERIFY(!p2.matches(dev));
    QVERIFY(p3.matches(dev));
    QVERIFY(!p4.matches(dev));

    Solid::Predicate p6 = Solid::Predicate::fromString("StorageVolume.usage == 'Other'");
    Solid::Predicate p7 = Solid::Predicate::fromString(QString("StorageVolume.usage == %1").arg((int)Solid::StorageVolume::Other));
    QVERIFY(!p6.matches(dev));
    QVERIFY(!p7.matches(dev));
    dev = Solid::Device("/org/kde/solid/fakehw/volume_part2_size_1024");
    QVERIFY(p6.matches(dev));
    QVERIFY(p7.matches(dev));

    Solid::Predicate p8 = Solid::Predicate::fromString("AudioInterface.deviceType == 'AudioInput|AudioOutput'");
    Solid::Predicate p9 = Solid::Predicate::fromString("AudioInterface.deviceType == 'AudioInput'");
    Solid::Predicate p10 = Solid::Predicate::fromString("AudioInterface.deviceType  & 'AudioInput'");
    Solid::Predicate p11 = Solid::Predicate::fromString("AudioInterface.deviceType  & 'foobar'");
    QVERIFY(!p8.matches(dev));
    QVERIFY(!p9.matches(dev));
    QVERIFY(!p10.matches(dev));
    QVERIFY(!p11.matches(dev));
    dev = Solid::Device("/org/kde/solid/fakehw/pci_8086_266e_oss_pcm_0");
    QVERIFY(p8.matches(dev));
    QVERIFY(!p9.matches(dev));
    QVERIFY(p10.matches(dev));
    QVERIFY(!p11.matches(dev));

    QString str_pred = "[[Processor.maxSpeed == 3201 AND Processor.canChangeFrequency == false] OR StorageVolume.mountPoint == '/media/blup']";
    // Since str_pred is canonicalized, fromString().toString() should be invariant
    QCOMPARE(Solid::Predicate::fromString(str_pred).toString(), str_pred);

    // Invalid predicate
    str_pred = "[StorageVolume.ignored == false AND OpticalDisc.isBlank == true AND OpticalDisc.discType & 'CdRecordable|CdRewritable']";
    QVERIFY(!Solid::Predicate::fromString(str_pred).isValid());

    QString parentUdi = "/org/kde/solid/fakehw/storage_model_solid_reader";
    Solid::DeviceInterface::Type ifaceType = Solid::DeviceInterface::Unknown;
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).size(), 1);
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).at(0),
              QString("/org/kde/solid/fakehw/volume_label_SOLIDMAN_BEGINS"));

    ifaceType = Solid::DeviceInterface::Processor;
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).size(), 0);

    parentUdi = "/org/kde/solid/fakehw/computer";
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).size(), 2);
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).at(0),
              QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(fakeManager->devicesFromQuery(parentUdi, ifaceType).at(1),
              QString("/org/kde/solid/fakehw/acpi_CPU1"));


    parentUdi.clear();
    ifaceType = Solid::DeviceInterface::Unknown;
    QList<Solid::Device> list;

    list = Solid::Device::listFromQuery(p1, parentUdi);
    QCOMPARE(list.size(), 2);
    QCOMPARE(list.at(0).udi(), QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(list.at(1).udi(), QString("/org/kde/solid/fakehw/acpi_CPU1"));

    list = Solid::Device::listFromQuery(p2, parentUdi);
    QCOMPARE(list.size(), 0);

    list = Solid::Device::listFromQuery(p3, parentUdi);
    QCOMPARE(list.size(), 2);
    QCOMPARE(list.at(0).udi(), QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(list.at(1).udi(), QString("/org/kde/solid/fakehw/acpi_CPU1"));

    list = Solid::Device::listFromQuery(p4, parentUdi);
    QCOMPARE(list.size(), 0);

    list = Solid::Device::listFromQuery("[Processor.canChangeFrequency==true AND Processor.number==1]",
                                                       parentUdi);
    QCOMPARE(list.size(), 1);
    QCOMPARE(list.at(0).udi(), QString("/org/kde/solid/fakehw/acpi_CPU1"));

    list = Solid::Device::listFromQuery("[Processor.number==1 OR IS StorageVolume]",
                                                       parentUdi);
    QCOMPARE(list.size(), 10);

    //make sure predicate case-insensitiveness is sane
    list = Solid::Device::listFromQuery("[Processor.number==1 or is StorageVolume]",
                                                       parentUdi);
    QCOMPARE(list.size(), 10);
    list = Solid::Device::listFromQuery("[Processor.number==1 oR Is StorageVolume]",
                                                       parentUdi);
    QCOMPARE(list.size(), 10);

    QSet<QString> set;
    QCOMPARE(list.size(), 10);
    set << "/org/kde/solid/fakehw/acpi_CPU1"
        << "/org/kde/solid/fakehw/platform_floppy_0_storage_virt_volume"
        << "/org/kde/solid/fakehw/volume_label_SOLIDMAN_BEGINS"
        << "/org/kde/solid/fakehw/volume_part1_size_993284096"
        << "/org/kde/solid/fakehw/volume_part2_size_1024"
        << "/org/kde/solid/fakehw/volume_part5_size_1048576"
        << "/org/kde/solid/fakehw/volume_uuid_5011"
        << "/org/kde/solid/fakehw/volume_uuid_c0ffee"
        << "/org/kde/solid/fakehw/volume_uuid_f00ba7"
        << "/org/kde/solid/fakehw/volume_uuid_feedface";
    QCOMPARE(set, to_string_set(list));

    list = Solid::Device::listFromQuery("[IS Processor OR IS StorageVolume]",
                                         parentUdi);
    QCOMPARE(list.size(), 11);
    set << "/org/kde/solid/fakehw/acpi_CPU0";
    QCOMPARE(set, to_string_set(list));

    ifaceType = Solid::DeviceInterface::Processor;
    list = Solid::Device::listFromType(ifaceType, parentUdi);
    QCOMPARE(list.size(), 2);
    QCOMPARE(list.at(0).udi(), QString("/org/kde/solid/fakehw/acpi_CPU0"));
    QCOMPARE(list.at(1).udi(), QString("/org/kde/solid/fakehw/acpi_CPU1"));

    ifaceType = Solid::DeviceInterface::Unknown;
    list = Solid::Device::listFromQuery("blup", parentUdi);
    QCOMPARE(list.size(), 0);
}

void SolidHwTest::testSetupTeardown()
{
    Solid::StorageAccess *access;
    {
        Solid::Device device("/org/kde/solid/fakehw/volume_part1_size_993284096");
        access = device.as<Solid::StorageAccess>();
    }

    QList<QVariant> args;
    QSignalSpy spy(access, SIGNAL(accessibilityChanged(bool,QString)));

    access->teardown();

    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toBool(), false);

    access->setup();

    QCOMPARE(spy.count(), 1);
    args = spy.takeFirst();
    QCOMPARE(args.at(0).toBool(), true);

}

void SolidHwTest::slotPropertyChanged(const QMap<QString,int> &changes)
{
    m_changesList << changes;
}


#include "solidhwtest.moc"

