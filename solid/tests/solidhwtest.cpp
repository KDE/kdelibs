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

#include <qtest_kde.h>

#include <kcomponentdata.h>
#include <kdebug.h>

#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/genericinterface.h>
#include <solid/processor.h>
#include <solid/volume.h>
#include <solid/predicate.h>
#include "solid/managerbase_p.h"

#include <fakemanager.h>
#include <fakedevice.h>

#ifndef FAKE_COMPUTER_XML
    #error "FAKE_COMPUTER_XML not set. An XML file describing a computer is required for this test"
#endif

QTEST_KDEMAIN_CORE( SolidHwTest )

void SolidHwTest::initTestCase()
{
    fakeManager = new FakeManager(0, QStringList(), FAKE_COMPUTER_XML);
    Solid::ManagerBasePrivate::_k_forcePreloadedBackend("Solid::Ifaces::DeviceManager", fakeManager);
}

void SolidHwTest::testAllDevices()
{
    Solid::DeviceList devices = Solid::DeviceManager::allDevices();

    // Verify that the framework reported correctly the devices available
    // in the backend.
    QSet<QString> expected_udis, received_udis;

    expected_udis = QSet<QString>::fromList( fakeManager->allDevices() );

    foreach ( Solid::Device dev , devices )
    {
        received_udis << dev.udi();
    }

    QCOMPARE( expected_udis, received_udis );
}

void SolidHwTest::testDeviceBasicFeatures()
{
    // Retrieve a valid Device object
    Solid::Device valid_dev( "/org/kde/solid/fakehw/storage_model_solid_writer" );

    QCOMPARE( valid_dev.isValid(), true );


    // A few attempts at creating invalid Device objects
    Solid::Device invalid_dev( "uhoh? doesn't exist, I guess" );
    QCOMPARE( invalid_dev.isValid(), false );
    invalid_dev = Solid::Device( QString() );
    QCOMPARE( invalid_dev.isValid(), false );
    invalid_dev = Solid::Device();
    QCOMPARE( invalid_dev.isValid(), false );



    QCOMPARE( valid_dev.udi(), QString( "/org/kde/solid/fakehw/storage_model_solid_writer" ) );
    QCOMPARE( invalid_dev.udi(), QString() );


    // Query properties
    QCOMPARE( valid_dev.as<Solid::GenericInterface>()->propertyExists("name"), true );
    QCOMPARE( valid_dev.as<Solid::GenericInterface>()->propertyExists("foo.bar"), false );
    QCOMPARE( (QObject*)invalid_dev.as<Solid::GenericInterface>(), (QObject*)0 );

    QCOMPARE( valid_dev.as<Solid::GenericInterface>()->property("name"), QVariant( "Solid IDE DVD Writer" ) );
    QVERIFY( !valid_dev.as<Solid::GenericInterface>()->property("foo.bar").isValid() );

    FakeDevice *fake_device = fakeManager->findDevice( "/org/kde/solid/fakehw/storage_model_solid_writer" );
    QMap<QString, QVariant> expected_properties = fake_device->allProperties();

    QCOMPARE( valid_dev.as<Solid::GenericInterface>()->allProperties(), expected_properties );


    // Query device interfaces
    QCOMPARE( valid_dev.queryDeviceInterface( Solid::DeviceInterface::Storage ), true );
    QCOMPARE( valid_dev.queryDeviceInterface( Solid::DeviceInterface::Cdrom ), true );
    QCOMPARE( valid_dev.queryDeviceInterface( Solid::DeviceInterface::Volume ), false );

    QCOMPARE( invalid_dev.queryDeviceInterface( Solid::DeviceInterface::Unknown ), false );
    QCOMPARE( invalid_dev.queryDeviceInterface( Solid::DeviceInterface::Storage ), false );


    // Query parent
    QCOMPARE( valid_dev.parentUdi(), QString( "/org/kde/solid/fakehw/pci_002_ide_1_0" ) );
    QCOMPARE( valid_dev.parent().udi(), Solid::Device( "/org/kde/solid/fakehw/pci_002_ide_1_0" ).udi() );

    QVERIFY( !invalid_dev.parent().isValid() );
    QVERIFY( invalid_dev.parentUdi().isEmpty() );


    // Query vendor/product
    QCOMPARE( valid_dev.vendor(), QString( "Acme Corporation" ) );
    QCOMPARE( valid_dev.product(), QString( "Solid IDE DVD Writer" ) );

    QCOMPARE( invalid_dev.vendor(), QString() );
    QCOMPARE( invalid_dev.product(), QString() );
}

void SolidHwTest::testManagerSignals()
{
    fakeManager->unplug( "/org/kde/solid/fakehw/acpi_CPU0" );

    // Heh, we missed a processor in this system ;-)
    // We're going to add this device, and check that the signal has been
    // properly emitted by the manager
    QSignalSpy added( Solid::DeviceManager::notifier(), SIGNAL( deviceAdded( QString ) ) );
    fakeManager->plug( "/org/kde/solid/fakehw/acpi_CPU0" );
    QCOMPARE( added.count(), 1 );
    QCOMPARE( added.at( 0 ).at( 0 ).toString(), QString( "/org/kde/solid/fakehw/acpi_CPU0" ) );

    // Moreover we check that the device is really available
    Solid::Device cpu( "/org/kde/solid/fakehw/acpi_CPU0" );
    QVERIFY( cpu.isValid() );


    // Finally we remove the device and spy the corresponding signal again
    QSignalSpy removed( Solid::DeviceManager::notifier(), SIGNAL( deviceRemoved( QString ) ) );
    fakeManager->unplug( "/org/kde/solid/fakehw/acpi_CPU0" );
    QCOMPARE( added.count(), 1 );
    QCOMPARE( added.at( 0 ).at( 0 ).toString(), QString( "/org/kde/solid/fakehw/acpi_CPU0" ) );

    // The Device object should become automatically invalid
    QVERIFY( !cpu.isValid() );

    // Restore original state
    fakeManager->plug( "/org/kde/solid/fakehw/acpi_CPU0" );
}

void SolidHwTest::testDeviceSignals()
{
    // A button is a nice device for testing state changes, isn't it?
    FakeDevice *fake = fakeManager->findDevice( "/org/kde/solid/fakehw/acpi_LID0" );
    Solid::Device device( "/org/kde/solid/fakehw/acpi_LID0" );

    // We'll spy our button
    connect( device.as<Solid::GenericInterface>(), SIGNAL( propertyChanged( const QMap<QString,int>& ) ),
             this, SLOT( slotPropertyChanged( const QMap<QString,int>& ) ) );
    QSignalSpy condition_raised( device.as<Solid::GenericInterface>(), SIGNAL( conditionRaised( QString, QString ) ) );

    fake->setProperty( "stateValue", true ); // The button is now pressed (modified property)
    fake->raiseCondition( "Lid Closed", "Why not?" ); // Since it's a LID we notify this change
    fake->setProperty( "hactar", 42 ); // We add a property
    fake->removeProperty( "hactar" ); // We remove a property

    // 3 property changes occurred in the device
    QCOMPARE( m_changesList.count(), 3 );

    QMap<QString,int> changes;

    // First one is a "PropertyModified" for "button.state"
    changes = m_changesList.at( 0 );
    QCOMPARE( changes.count(), 1 );
    QVERIFY( changes.contains( "stateValue" ) );
    QCOMPARE( changes["stateValue"], (int)Solid::GenericInterface::PropertyModified );

    // Second one is a "PropertyAdded" for "hactar"
    changes = m_changesList.at( 1 );
    QCOMPARE( changes.count(), 1 );
    QVERIFY( changes.contains( "hactar" ) );
    QCOMPARE( changes["hactar"], (int)Solid::GenericInterface::PropertyAdded );

    // Third one is a "PropertyRemoved" for "hactar"
    changes = m_changesList.at( 2 );
    QCOMPARE( changes.count(), 1 );
    QVERIFY( changes.contains( "hactar" ) );
    QCOMPARE( changes["hactar"], (int)Solid::GenericInterface::PropertyRemoved );



    // Only one condition has been raised in the device
    QCOMPARE( condition_raised.count(), 1 );

    // It must be identical to the condition we raised by hand
    QCOMPARE( condition_raised.at( 0 ).at( 0 ).toString(), QString( "Lid Closed" ) );
    QCOMPARE( condition_raised.at( 0 ).at( 1 ).toString(), QString( "Why not?" ) );
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
    Solid::Device cpu( "/org/kde/solid/fakehw/acpi_CPU0" );

    Solid::DeviceInterface *iface = cpu.asDeviceInterface( Solid::DeviceInterface::Processor );
    Solid::Processor *processor = cpu.as<Solid::Processor>();

    QVERIFY( cpu.queryDeviceInterface( Solid::DeviceInterface::Processor ) );
    QVERIFY( iface!=0 );
    QCOMPARE( iface, processor );

    Solid::Device cpu2("/org/kde/solid/fakehw/acpi_CPU0");
    QCOMPARE(cpu.as<Solid::Processor>(), cpu2.as<Solid::Processor>());
    QCOMPARE(cpu.as<Solid::GenericInterface>(), cpu2.as<Solid::GenericInterface>());
}

void SolidHwTest::testDeviceInterfaceIntrospection_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("value");

    QTest::newRow("DeviceInterface: Unknown") << "Unknown" << (int)Solid::DeviceInterface::Unknown;
    QTest::newRow("DeviceInterface: Processor") << "Processor" << (int)Solid::DeviceInterface::Processor;
    QTest::newRow("DeviceInterface: Block") << "Block" << (int)Solid::DeviceInterface::Block;
    QTest::newRow("DeviceInterface: Storage") << "Storage" << (int)Solid::DeviceInterface::Storage;
    QTest::newRow("DeviceInterface: Cdrom") << "Cdrom" << (int)Solid::DeviceInterface::Cdrom;
    QTest::newRow("DeviceInterface: Volume") << "Volume" << (int)Solid::DeviceInterface::Volume;
    QTest::newRow("DeviceInterface: OpticalDisc") << "OpticalDisc" << (int)Solid::DeviceInterface::OpticalDisc;
    QTest::newRow("DeviceInterface: Camera") << "Camera" << (int)Solid::DeviceInterface::Camera;
    QTest::newRow("DeviceInterface: PortableMediaPlayer") << "PortableMediaPlayer" << (int)Solid::DeviceInterface::PortableMediaPlayer;
    QTest::newRow("DeviceInterface: NetworkHw") << "NetworkHw" << (int)Solid::DeviceInterface::NetworkHw;
    QTest::newRow("DeviceInterface: AcAdapter") << "AcAdapter" << (int)Solid::DeviceInterface::AcAdapter;
    QTest::newRow("DeviceInterface: Battery") << "Battery" << (int)Solid::DeviceInterface::Battery;
    QTest::newRow("DeviceInterface: Button") << "Button" << (int)Solid::DeviceInterface::Button;
    QTest::newRow("DeviceInterface: Display") << "Display" << (int)Solid::DeviceInterface::Display;
    QTest::newRow("DeviceInterface: AudioHw") << "AudioHw" << (int)Solid::DeviceInterface::AudioHw;
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

static QSet<QString> to_string_set(const Solid::DeviceList &list)
{
    QSet<QString> res;

    foreach (const Solid::Device &device, list) {
        res << device.udi();
    }

    return res;
}

void SolidHwTest::testPredicate()
{
    Solid::Device dev( "/org/kde/solid/fakehw/acpi_CPU0" );

    Solid::Predicate p1 = Solid::Predicate( Solid::DeviceInterface::Processor, "maxSpeed", 3200 )
                        & Solid::Predicate( Solid::DeviceInterface::Processor, "canThrottle", true );
    Solid::Predicate p2 = Solid::Predicate( Solid::DeviceInterface::Processor, "maxSpeed", 3200 )
                        & Solid::Predicate( Solid::DeviceInterface::Processor, "canThrottle", false );
    Solid::Predicate p3 = Solid::Predicate( Solid::DeviceInterface::Processor, "maxSpeed", 3201 )
                        | Solid::Predicate( Solid::DeviceInterface::Processor, "canThrottle", true );
    Solid::Predicate p4 = Solid::Predicate( Solid::DeviceInterface::Processor, "maxSpeed", 3201 )
                        | Solid::Predicate( Solid::DeviceInterface::Processor, "canThrottle", false );
    Solid::Predicate p5 = Solid::Predicate::fromString( "[ [ Processor.maxSpeed == 3201 AND Processor.canThrottle == false ] OR Volume.mountPoint == '/media/blup' ]" );

    QVERIFY( p1.matches( dev ) );
    QVERIFY( !p2.matches( dev ) );
    QVERIFY( p3.matches( dev ) );
    QVERIFY( !p4.matches( dev ) );

    Solid::Predicate p6 = Solid::Predicate::fromString( "Volume.usage == 'Other'" );
    Solid::Predicate p7 = Solid::Predicate::fromString( QString("Volume.usage == %1").arg((int)Solid::Volume::Other) );
    QVERIFY( !p6.matches( dev ) );
    QVERIFY( !p7.matches( dev ) );
    dev = Solid::Device( "/org/kde/solid/fakehw/volume_part2_size_1024" );
    QVERIFY( p6.matches( dev ) );
    QVERIFY( p7.matches( dev ) );

    Solid::Predicate p8 = Solid::Predicate::fromString( "AudioHw.deviceType == 'AudioInput|AudioOutput'" );
    Solid::Predicate p9 = Solid::Predicate::fromString( "AudioHw.deviceType == 'AudioInput'" );
    Solid::Predicate p10 = Solid::Predicate::fromString( "AudioHw.deviceType & 'AudioInput'" );
    QVERIFY( !p8.matches( dev ) );
    QVERIFY( !p9.matches( dev ) );
    QVERIFY( !p10.matches( dev ) );
    dev = Solid::Device( "/org/kde/solid/fakehw/pci_8086_266e_oss_pcm_0" );
    QVERIFY( p8.matches( dev ) );
    QVERIFY( !p9.matches( dev ) );
    QVERIFY( p10.matches( dev ) );

    QString str_pred = "[ [ Processor.maxSpeed == 3201 AND Processor.canThrottle == false ] OR Volume.mountPoint == '/media/blup' ]";
    // Since str_pred is canonicalized, fromString().toString() should be invariant
    QCOMPARE( Solid::Predicate::fromString( str_pred ).toString(), str_pred );


    QString parentUdi = "/org/kde/solid/fakehw/storage_model_solid_reader";
    Solid::DeviceInterface::Type ifaceType = Solid::DeviceInterface::Unknown;
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, ifaceType ).size(), 1 );
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, ifaceType ).at( 0 ),
              QString( "/org/kde/solid/fakehw/volume_label_SOLIDMAN_BEGINS" ) );

    ifaceType = Solid::DeviceInterface::Processor;
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, ifaceType ).size(), 0 );

    parentUdi = "/org/kde/solid/fakehw/computer";
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, ifaceType ).size(), 2 );
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, ifaceType ).at( 0 ),
              QString( "/org/kde/solid/fakehw/acpi_CPU0" ) );
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, ifaceType ).at( 1 ),
              QString( "/org/kde/solid/fakehw/acpi_CPU1" ) );


    parentUdi = QString();
    ifaceType = Solid::DeviceInterface::Unknown;
    Solid::DeviceList list;

    list = Solid::DeviceManager::findDevicesFromQuery( p1, parentUdi );
    QCOMPARE( list.size(), 2 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/org/kde/solid/fakehw/acpi_CPU0" ) );
    QCOMPARE( list.at( 1 ).udi(), QString( "/org/kde/solid/fakehw/acpi_CPU1" ) );

    list = Solid::DeviceManager::findDevicesFromQuery( p2, parentUdi );
    QCOMPARE( list.size(), 0 );

    list = Solid::DeviceManager::findDevicesFromQuery( p3, parentUdi );
    QCOMPARE( list.size(), 2 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/org/kde/solid/fakehw/acpi_CPU0" ) );
    QCOMPARE( list.at( 1 ).udi(), QString( "/org/kde/solid/fakehw/acpi_CPU1" ) );

    list = Solid::DeviceManager::findDevicesFromQuery( p4, parentUdi );
    QCOMPARE( list.size(), 0 );

    list = Solid::DeviceManager::findDevicesFromQuery( "[Processor.canThrottle==true AND Processor.number==1]",
                                                       parentUdi );
    QCOMPARE( list.size(), 1 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/org/kde/solid/fakehw/acpi_CPU1" ) );

    list = Solid::DeviceManager::findDevicesFromQuery( "[Processor.number==1 OR IS Volume]",
                                                       parentUdi );

    QSet<QString> set;

    QCOMPARE( list.size(), 10 );
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
    QCOMPARE( set, to_string_set( list ) );

    list = Solid::DeviceManager::findDevicesFromQuery( "[IS Processor OR IS Volume]",
                                         parentUdi );
    QCOMPARE( list.size(), 11 );
    set << "/org/kde/solid/fakehw/acpi_CPU0";
    QCOMPARE( set, to_string_set( list ) );

    ifaceType = Solid::DeviceInterface::Processor;
    list = Solid::DeviceManager::findDevicesFromQuery( ifaceType, parentUdi );
    QCOMPARE( list.size(), 2 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/org/kde/solid/fakehw/acpi_CPU0" ) );
    QCOMPARE( list.at( 1 ).udi(), QString( "/org/kde/solid/fakehw/acpi_CPU1" ) );

    ifaceType = Solid::DeviceInterface::Unknown;
    list = Solid::DeviceManager::findDevicesFromQuery( "blup", parentUdi );
    QCOMPARE( list.size(), 0 );
}

void SolidHwTest::slotPropertyChanged( const QMap<QString,int> &changes )
{
    m_changesList << changes;
}

#include "solidhwtest.moc"

