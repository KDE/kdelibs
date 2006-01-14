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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "kdehwtest.h"

#include <qtest_kde.h>

#include <kinstance.h>
#include <kdebug.h>

#include "devicemanager.h"
#include "device.h"
#include "fakemanager.h"
#include "fakedevice.h"

QTEST_KDEMAIN( KdeHwTest, NoGUI )

void KdeHwTest::initTestCase()
{
    fakeManager = new FakeManager();
    KDEHW::DeviceManager::selfForceBackend( fakeManager );

    FakeDevice *dev;

    // Initialise a fake system
    dev = fakeManager->newDevice( "/fake/computer" );
    dev->setProperty( "system.formfactor", "laptop" );
    dev->setProperty( "powermanagement.isEnabled", true );
    dev->setProperty( "info.product", "Black Ice" );
    dev->setProperty( "info.vendor", "Gibson Inc." );

    dev = fakeManager->newDevice( "/fake/storage_SONA_CD22U" );
    dev->setProperty( "info.product", "CD22U" );
    dev->setProperty( "info.vendor", "Sona Inc." );
    dev->setProperty( "info.parent", "/fake/computer" );
    dev->addCapability( "storage" );
    dev->addCapability( "storage.cdrom" );

    dev = fakeManager->newDevice( "/fake/volume_label_SOLIDMAN_THE_MOVIE" );
    dev->setProperty( "info.product", "SOLIDMAN_THE_MOVIE" );
    dev->setProperty( "info.parent", "/fake/storage_SONA_CD22U" );
    dev->setProperty( "volume.label", "SOLIDMAN_THE_MOVIE" );
    dev->setProperty( "volume.disc.type", "dvd_rom" );
    dev->setProperty( "volume.disc.is_videodvd", true );
    dev->addCapability( "volume" );
    dev->addCapability( "volume.disc" );

    dev = fakeManager->newDevice( "/fake/acpi_LID0" );
    dev->setProperty( "info.product", "SOLIDMAN_THE_MOVIE" );
    dev->setProperty( "info.parent", "/fake/computer" );
    dev->setProperty( "button.type", "lid" );
    dev->setProperty( "button.has_state", true );
    dev->setProperty( "button.state", false );
    dev->addCapability( "button" );
}

void KdeHwTest::testAllDevices()
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();

    KDEHW::DeviceList devices = manager.allDevices();

    // Verify that the framework reported correctly the devices available
    // in the backend.
    QSet<QString> expected_udis, received_udis;

    expected_udis << "/fake/computer" << "/fake/storage_SONA_CD22U"
                  << "/fake/volume_label_SOLIDMAN_THE_MOVIE"
                  << "/fake/acpi_LID0";

    foreach ( KDEHW::Device dev , devices )
    {
        received_udis << dev.udi();
    }

    QCOMPARE( expected_udis, received_udis );
}

void KdeHwTest::testDeviceExists()
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();

    QCOMPARE( manager.deviceExists( "/fake/acpi_LID0" ), true );
    QCOMPARE( manager.deviceExists( "/fake/volume_label_SOLIDMAN_THE_MOVIE" ), true );

    // Note the extra space
    QCOMPARE( manager.deviceExists( "/fake/computer " ), false );
    QCOMPARE( manager.deviceExists( "#'({(à]" ), false );
    QCOMPARE( manager.deviceExists( QString() ), false );
}

void KdeHwTest::testDeviceBasicFeatures()
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();

    // Retrieve a valid Device object
    KDEHW::Device valid_dev = manager.findDevice( "/fake/storage_SONA_CD22U" );

    QCOMPARE( valid_dev.isValid(), true );


    // A few attempts at creating invalid Device objects
    KDEHW::Device invalid_dev = manager.findDevice( "uhoh? doesn't exist, I guess" );
    QCOMPARE( invalid_dev.isValid(), false );
    invalid_dev = manager.findDevice( QString() );
    QCOMPARE( invalid_dev.isValid(), false );
    invalid_dev = KDEHW::Device();
    QCOMPARE( invalid_dev.isValid(), false );



    QCOMPARE( valid_dev.udi(), QString( "/fake/storage_SONA_CD22U" ) );
    QCOMPARE( invalid_dev.udi(), QString() );


    // Query properties
    QCOMPARE( valid_dev.propertyExists("info.product"), true );
    QCOMPARE( valid_dev.propertyExists("foo.bar"), false );
    QCOMPARE( invalid_dev.propertyExists("info.product"), false );
    QCOMPARE( invalid_dev.propertyExists("foo.bar"), false );

    QCOMPARE( valid_dev.property("info.product"), QVariant( "CD22U" ) );
    QVERIFY( !valid_dev.property("foo.bar").isValid() );
    QVERIFY( !invalid_dev.property("info.product").isValid() );
    QVERIFY( !invalid_dev.property("foo.bar").isValid() );

    QMap<QString, QVariant> expected_properties;
    expected_properties["info.product"] = "CD22U";
    expected_properties["info.vendor"] = "Sona Inc.";
    expected_properties["info.parent"] = "/fake/computer";
    QCOMPARE( valid_dev.allProperties(), expected_properties );
    QVERIFY( invalid_dev.allProperties().isEmpty() );


    // Query capabilities
    QCOMPARE( valid_dev.queryCapability( "storage" ), true );
    QCOMPARE( valid_dev.queryCapability( "storage.cdrom" ), true );
    QCOMPARE( valid_dev.queryCapability( "volume" ), false );

    QCOMPARE( invalid_dev.queryCapability( QString() ), false );
    QCOMPARE( invalid_dev.queryCapability( "storage" ), false );
}

void KdeHwTest::testDeviceLocking()
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();

    KDEHW::Device device = manager.findDevice( "/fake/computer" );


    // Test locking on a device that refuses it
    fakeManager->findDevice( "/fake/computer" )->setBroken( true );
    QCOMPARE( device.lock( "need a reason?" ), false );
    QCOMPARE( device.lock( "really need one!?" ), false );
    QCOMPARE( device.unlock(), false );
    QCOMPARE( device.unlock(), false );


    // Test locking on a "normal" device
    fakeManager->findDevice( "/fake/computer" )->setBroken( false );
    QCOMPARE( device.lock( "sure I have a good reason" ), true );
    QCOMPARE( device.lock( "hope it won't fail" ), false );
    QCOMPARE( device.unlock(), true );
    QCOMPARE( device.unlock(), false );


    // Test locking on an invalid Device object
    device = KDEHW::Device();
    QCOMPARE( device.lock( "won't work!" ), false );
    QCOMPARE( device.lock( "..." ), false );
    QCOMPARE( device.unlock(), false );
    QCOMPARE( device.unlock(), false );
}



void KdeHwTest::testManagerSignals()
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();

    FakeDevice *dev;


    // Heh, we missed a processor in this system ;-)
    // We're going to add this device, and check that the signal has been
    // properly emitted by the manager
    QSignalSpy added( &manager, SIGNAL( deviceAdded( QString ) ) );
    dev = fakeManager->newDevice( "/fake/acpi_CPU0" );
    QCOMPARE( added.count(), 1 );
    QCOMPARE( added.at( 0 ).at( 0 ).toString(), QString( "/fake/acpi_CPU0" ) );

    // Moreover we check that the device is really available
    KDEHW::Device cpu = manager.findDevice( "/fake/acpi_CPU0" );
    QVERIFY( cpu.isValid() );



    // Now we add a capability to the newly created device, and spy the signal
    QSignalSpy new_capability( &manager, SIGNAL( newCapability( QString, QString ) ) );
    dev->addCapability( "processor" );
    QCOMPARE( new_capability.count(), 1 );
    QCOMPARE( new_capability.at( 0 ).at( 0 ).toString(), QString( "/fake/acpi_CPU0" ) );
    QCOMPARE( new_capability.at( 0 ).at( 1 ).toString(), QString( "processor" ) );

    // We also check that the Device object noticed it
    QVERIFY( cpu.queryCapability( "processor" ) );



    // Finally we remove the device and spy the corresponding signal again
    QSignalSpy removed( &manager, SIGNAL( deviceRemoved( QString ) ) );
    fakeManager->deleteDevice( "/fake/acpi_CPU0" );
    QCOMPARE( added.count(), 1 );
    QCOMPARE( added.at( 0 ).at( 0 ).toString(), QString( "/fake/acpi_CPU0" ) );

    // The Device object should become automatically invalid
    QVERIFY( !cpu.isValid() );
}

void KdeHwTest::testDeviceSignals()
{
    KDEHW::DeviceManager &manager = KDEHW::DeviceManager::self();


    // A button is a nice device for testing state changes, isn't it?
    FakeDevice *fake = fakeManager->findDevice( "/fake/acpi_LID0" );
    KDEHW::Device device = manager.findDevice( "/fake/acpi_LID0" );



    // We'll spy our button
    QSignalSpy property_changed( &device, SIGNAL( propertyChanged( QString, int ) ) );
    QSignalSpy condition_raised( &device, SIGNAL( conditionRaised( QString, QString ) ) );

    fake->setProperty( "button.state", true ); // The button is now pressed (modified property)
    fake->raiseCondition( "Lid Closed", "Why not?" ); // Since it's a LID we notify this change
    fake->setProperty( "hactar", 42 ); // We add a property
    fake->removeProperty( "hactar" ); // We remove a property



    // 3 property changes occured in the device
    QCOMPARE( property_changed.count(), 3 );

    // First one is a "PropertyModified" for "button.state"
    QCOMPARE( property_changed.at( 0 ).at( 0 ).toString(), QString( "button.state" ) );
    QCOMPARE( property_changed.at( 0 ).at( 1 ), QVariant( KDEHW::PropertyModified ) );

    // Second one is a "PropertyAdded" for "hactar"
    QCOMPARE( property_changed.at( 1 ).at( 0 ).toString(), QString( "hactar" ) );
    QCOMPARE( property_changed.at( 1 ).at( 1 ), QVariant( KDEHW::PropertyAdded ) );

    // Third one is a "PropertyRemoved" for "hactar"
    QCOMPARE( property_changed.at( 2 ).at( 0 ).toString(), QString( "hactar" ) );
    QCOMPARE( property_changed.at( 2 ).at( 1 ), QVariant( KDEHW::PropertyRemoved ) );



    // Only one condition has been raised in the device
    QCOMPARE( condition_raised.count(), 1 );

    // It must be identical to the condition we raised by hand
    QCOMPARE( condition_raised.at( 0 ).at( 0 ).toString(), QString( "Lid Closed" ) );
    QCOMPARE( condition_raised.at( 0 ).at( 1 ).toString(), QString( "Why not?" ) );
}

#include "kdehwtest.moc"

