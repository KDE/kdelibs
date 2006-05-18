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

#include "solidtest.h"

#include <qtest_kde.h>

#include <kinstance.h>
#include <kdebug.h>

#include <solid/devicemanager.h>
#include <solid/device.h>
#include <solid/processor.h>
#include <solid/predicate.h>
#include "fakeprocessor.h"
#include "fakemanager.h"
#include "fakedevice.h"

QTEST_KDEMAIN( SolidTest, NoGUI )

void SolidTest::initTestCase()
{
    fakeManager = new FakeManager();
    Solid::DeviceManager::selfForceBackend( fakeManager );

    FakeDevice *dev;

    // Initialise a fake system
    dev = fakeManager->newDevice( "/fake/computer" );
    dev->setProperty( "system.formfactor", "laptop" );
    dev->setProperty( "powermanagement.isEnabled", true );
    dev->setProperty( "info.product", "Black Ice" );
    dev->setProperty( "info.vendor", "Gibson Inc." );

    dev = fakeManager->newDevice( "/fake/storage_SONA_CD22U" );
    dev->setParent( "/fake/computer" );
    dev->setProperty( "info.product", "CD22U" );
    dev->setProperty( "info.vendor", "Sona Inc." );
    dev->setProperty( "info.parent", "/fake/computer" );
    dev->addCapability( Solid::Ifaces::Capability::Storage );
    dev->addCapability( Solid::Ifaces::Capability::Cdrom );

    dev = fakeManager->newDevice( "/fake/volume_label_SOLIDMAN_THE_MOVIE" );
    dev->setParent( "/fake/storage_SONA_CD22U" );
    dev->setProperty( "info.product", "SOLIDMAN_THE_MOVIE" );
    dev->setProperty( "info.parent", "/fake/storage_SONA_CD22U" );
    dev->setProperty( "volume.label", "SOLIDMAN_THE_MOVIE" );
    dev->setProperty( "volume.disc.type", "dvd_rom" );
    dev->setProperty( "volume.disc.is_videodvd", true );
    dev->addCapability( Solid::Ifaces::Capability::Volume );
    dev->addCapability( Solid::Ifaces::Capability::OpticalDisc );

    dev = fakeManager->newDevice( "/fake/acpi_LID0" );
    dev->setParent( "/fake/computer" );
    dev->setProperty( "info.product", "SOLIDMAN_THE_MOVIE" );
    dev->setProperty( "info.parent", "/fake/computer" );
    dev->setProperty( "button.type", "lid" );
    dev->setProperty( "button.has_state", true );
    dev->setProperty( "button.state", false );
    //dev->addCapability( "button" );
}

void SolidTest::testAllDevices()
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    Solid::DeviceList devices = manager.allDevices();

    // Verify that the framework reported correctly the devices available
    // in the backend.
    QSet<QString> expected_udis, received_udis;

    expected_udis << "/fake/computer" << "/fake/storage_SONA_CD22U"
                  << "/fake/volume_label_SOLIDMAN_THE_MOVIE"
                  << "/fake/acpi_LID0";

    foreach ( Solid::Device dev , devices )
    {
        received_udis << dev.udi();
    }

    QCOMPARE( expected_udis, received_udis );
}

void SolidTest::testDeviceExists()
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    QCOMPARE( manager.deviceExists( "/fake/acpi_LID0" ), true );
    QCOMPARE( manager.deviceExists( "/fake/volume_label_SOLIDMAN_THE_MOVIE" ), true );

    // Note the extra space
    QCOMPARE( manager.deviceExists( "/fake/computer " ), false );
    QCOMPARE( manager.deviceExists( "#'({(à]" ), false );
    QCOMPARE( manager.deviceExists( QString() ), false );
}

void SolidTest::testDeviceBasicFeatures()
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    // Retrieve a valid Device object
    Solid::Device valid_dev = manager.findDevice( "/fake/storage_SONA_CD22U" );

    QCOMPARE( valid_dev.isValid(), true );


    // A few attempts at creating invalid Device objects
    Solid::Device invalid_dev = manager.findDevice( "uhoh? doesn't exist, I guess" );
    QCOMPARE( invalid_dev.isValid(), false );
    invalid_dev = manager.findDevice( QString() );
    QCOMPARE( invalid_dev.isValid(), false );
    invalid_dev = Solid::Device();
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
    QCOMPARE( valid_dev.queryCapability( Solid::Ifaces::Capability::Storage ), true );
    QCOMPARE( valid_dev.queryCapability( Solid::Ifaces::Capability::Cdrom ), true );
    QCOMPARE( valid_dev.queryCapability( Solid::Ifaces::Capability::Volume ), false );

    QCOMPARE( invalid_dev.queryCapability( Solid::Ifaces::Capability::Unknown ), false );
    QCOMPARE( invalid_dev.queryCapability( Solid::Ifaces::Capability::Storage ), false );


    // Query parent
    QCOMPARE( valid_dev.parentUdi(), QString( "/fake/computer" ) );
    QCOMPARE( valid_dev.parent().udi(), manager.findDevice( "/fake/computer" ).udi() );

    QVERIFY( !invalid_dev.parent().isValid() );
    QVERIFY( invalid_dev.parentUdi().isEmpty() );


    // Query vendor/product
    QCOMPARE( valid_dev.vendor(), QString( "Sona Inc." ) );
    QCOMPARE( valid_dev.product(), QString( "CD22U" ) );

    QCOMPARE( invalid_dev.vendor(), QString() );
    QCOMPARE( invalid_dev.product(), QString() );
}

void SolidTest::testDeviceLocking()
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    Solid::Device device = manager.findDevice( "/fake/computer" );


    // Test locking on a device that refuses it
    fakeManager->findDevice( "/fake/computer" )->setBroken( true );
    QCOMPARE( device.lock( "need a reason?" ), false );
    QVERIFY( !device.isLocked() );
    QCOMPARE( device.lockReason(), QString() );
    QCOMPARE( device.lock( "really need one!?" ), false );
    QVERIFY( !device.isLocked() );
    QCOMPARE( device.lockReason(), QString() );
    QCOMPARE( device.unlock(), false );
    QCOMPARE( device.unlock(), false );


    // Test locking on a "normal" device
    fakeManager->findDevice( "/fake/computer" )->setBroken( false );
    QCOMPARE( device.lock( "sure I have a good reason" ), true );
    QVERIFY( device.isLocked() );
    QCOMPARE( device.lockReason(), QString( "sure I have a good reason" ) );
    QCOMPARE( device.lock( "hope it won't fail" ), false );
    QVERIFY( device.isLocked() );
    QCOMPARE( device.unlock(), true );
    QVERIFY( !device.isLocked() );
    QCOMPARE( device.unlock(), false );
    QVERIFY( !device.isLocked() );


    // Test locking on an invalid Device object
    device = Solid::Device();
    QCOMPARE( device.lock( "won't work!" ), false );
    QVERIFY( !device.isLocked() );
    QCOMPARE( device.lockReason(), QString() );
    QCOMPARE( device.lock( "..." ), false );
    QVERIFY( !device.isLocked() );
    QCOMPARE( device.lockReason(), QString() );
    QCOMPARE( device.unlock(), false );
    QCOMPARE( device.unlock(), false );
}

void SolidTest::testManagerSignals()
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    FakeDevice *dev;


    // Heh, we missed a processor in this system ;-)
    // We're going to add this device, and check that the signal has been
    // properly emitted by the manager
    QSignalSpy added( &manager, SIGNAL( deviceAdded( QString ) ) );
    dev = fakeManager->newDevice( "/fake/acpi_CPU0" );
    QCOMPARE( added.count(), 1 );
    QCOMPARE( added.at( 0 ).at( 0 ).toString(), QString( "/fake/acpi_CPU0" ) );

    // Moreover we check that the device is really available
    Solid::Device cpu = manager.findDevice( "/fake/acpi_CPU0" );
    QVERIFY( cpu.isValid() );



    // Now we add a capability to the newly created device, and spy the signal
    QSignalSpy new_capability( &manager, SIGNAL( newCapability( QString, int ) ) );
    dev->addCapability( Solid::Ifaces::Capability::Processor );
    QCOMPARE( new_capability.count(), 1 );
    QCOMPARE( new_capability.at( 0 ).at( 0 ).toString(), QString( "/fake/acpi_CPU0" ) );
    QCOMPARE( new_capability.at( 0 ).at( 1 ), QVariant( Solid::Ifaces::Capability::Processor ) );

    // We also check that the Device object noticed it
    QVERIFY( cpu.queryCapability( Solid::Ifaces::Capability::Processor ) );



    // Finally we remove the device and spy the corresponding signal again
    QSignalSpy removed( &manager, SIGNAL( deviceRemoved( QString ) ) );
    fakeManager->deleteDevice( "/fake/acpi_CPU0" );
    QCOMPARE( added.count(), 1 );
    QCOMPARE( added.at( 0 ).at( 0 ).toString(), QString( "/fake/acpi_CPU0" ) );

    // The Device object should become automatically invalid
    QVERIFY( !cpu.isValid() );
}

void SolidTest::testDeviceSignals()
{
    Solid::DeviceManager &manager = Solid::DeviceManager::self();


    // A button is a nice device for testing state changes, isn't it?
    FakeDevice *fake = fakeManager->findDevice( "/fake/acpi_LID0" );
    Solid::Device device = manager.findDevice( "/fake/acpi_LID0" );

    // We'll spy our button
    connect( &device, SIGNAL( propertyChanged( const QMap<QString,int>& ) ),
             this, SLOT( slotPropertyChanged( const QMap<QString,int>& ) ) );
    QSignalSpy condition_raised( &device, SIGNAL( conditionRaised( QString, QString ) ) );

    fake->setProperty( "button.state", true ); // The button is now pressed (modified property)
    fake->raiseCondition( "Lid Closed", "Why not?" ); // Since it's a LID we notify this change
    fake->setProperty( "hactar", 42 ); // We add a property
    fake->removeProperty( "hactar" ); // We remove a property

    // 3 property changes occured in the device
    QCOMPARE( m_changesList.count(), 3 );

    QMap<QString,int> changes;

    // First one is a "PropertyModified" for "button.state"
    changes = m_changesList.at( 0 );
    QCOMPARE( changes.count(), 1 );
    QVERIFY( changes.contains( "button.state" ) );
    QCOMPARE( changes["button.state"], (int)Solid::Device::PropertyModified );

    // Second one is a "PropertyAdded" for "hactar"
    changes = m_changesList.at( 1 );
    QCOMPARE( changes.count(), 1 );
    QVERIFY( changes.contains( "hactar" ) );
    QCOMPARE( changes["hactar"], (int)Solid::Device::PropertyAdded );

    // Third one is a "PropertyRemoved" for "hactar"
    changes = m_changesList.at( 2 );
    QCOMPARE( changes.count(), 1 );
    QVERIFY( changes.contains( "hactar" ) );
    QCOMPARE( changes["hactar"], (int)Solid::Device::PropertyRemoved );



    // Only one condition has been raised in the device
    QCOMPARE( condition_raised.count(), 1 );

    // It must be identical to the condition we raised by hand
    QCOMPARE( condition_raised.at( 0 ).at( 0 ).toString(), QString( "Lid Closed" ) );
    QCOMPARE( condition_raised.at( 0 ).at( 1 ).toString(), QString( "Why not?" ) );
}

void SolidTest::testDeviceCapabilities()
{
    FakeDevice *fake = fakeManager->newDevice( "/fake/acpi_CPU0" );
    fake->setParent( "/fake/computer" );
    fake->addCapability( Solid::Capability::Processor );
    QVERIFY( fake->asCapability( Solid::Capability::Processor ) != 0 );

    Solid::DeviceManager &manager = Solid::DeviceManager::self();
    Solid::Device cpu = manager.findDevice( "/fake/acpi_CPU0" );

    Solid::Capability *iface = cpu.asCapability( Solid::Capability::Processor );
    Solid::Processor *processor = cpu.as<Solid::Processor>();

    QVERIFY( cpu.queryCapability( Solid::Capability::Processor ) );
    QVERIFY( iface!=0 );
    QCOMPARE( iface, processor );
}

void SolidTest::testPredicate()
{
    FakeDevice *fake = fakeManager->findDevice( "/fake/acpi_CPU0" );

    Solid::Predicate p1 = Solid::Predicate( Solid::Capability::Processor, "maxSpeed", 3200 )
                        & Solid::Predicate( Solid::Capability::Processor, "canThrottle", true );
    Solid::Predicate p2 = Solid::Predicate( Solid::Capability::Processor, "maxSpeed", 3200 )
                        & Solid::Predicate( Solid::Capability::Processor, "canThrottle", false );
    Solid::Predicate p3 = Solid::Predicate( Solid::Capability::Processor, "maxSpeed", 3201 )
                        | Solid::Predicate( Solid::Capability::Processor, "canThrottle", true );
    Solid::Predicate p4 = Solid::Predicate( Solid::Capability::Processor, "maxSpeed", 3201 )
                        | Solid::Predicate( Solid::Capability::Processor, "canThrottle", false );
    Solid::Predicate p5 = Solid::Predicate::fromString( "[ [ Processor.maxSpeed == 3201 AND Processor.canThrottle == false ] OR Volume.mountPoint == '/media/blup' ]" );

    QVERIFY( p1.matches( fake ) );
    QVERIFY( !p2.matches( fake ) );
    QVERIFY( p3.matches( fake ) );
    QVERIFY( !p4.matches( fake ) );



    QString str_pred = "[ [ Processor.maxSpeed == 3201 AND Processor.canThrottle == false ] OR Volume.mountPoint == '/media/blup' ]";
    // Since str_pred is canonicalized, fromString().toString() should be invariant
    QCOMPARE( Solid::Predicate::fromString( str_pred ).toString(), str_pred );




    QString parentUdi = "/fake/storage_SONA_CD22U";
    Solid::Capability::Type capability = Solid::Capability::Unknown;
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, capability ).size(), 1 );
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, capability ).at( 0 ),
              QString( "/fake/volume_label_SOLIDMAN_THE_MOVIE" ) );

    capability = Solid::Capability::Processor;
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, capability ).size(), 0 );

    parentUdi = "/fake/computer";
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, capability ).size(), 1 );
    QCOMPARE( fakeManager->devicesFromQuery( parentUdi, capability ).at( 0 ),
              QString( "/fake/acpi_CPU0" ) );



    Solid::DeviceManager &manager = Solid::DeviceManager::self();

    parentUdi = QString();
    capability = Solid::Capability::Unknown;
    Solid::DeviceList list;


    list = manager.findDevicesFromQuery( parentUdi, capability, p1 );
    QCOMPARE( list.size(), 1 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/fake/acpi_CPU0" ) );

    list = manager.findDevicesFromQuery( parentUdi, capability, p2 );
    QCOMPARE( list.size(), 0 );

    list = manager.findDevicesFromQuery( parentUdi, capability, p3 );
    QCOMPARE( list.size(), 1 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/fake/acpi_CPU0" ) );

    list = manager.findDevicesFromQuery( parentUdi, capability, p4 );
    QCOMPARE( list.size(), 0 );

    list = manager.findDevicesFromQuery( parentUdi, capability,
                                         "[Processor.canThrottle==true AND Processor.number==1]" );
    QCOMPARE( list.size(), 1 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/fake/acpi_CPU0" ) );

    capability = Solid::Capability::Processor;
    list = manager.findDevicesFromQuery( parentUdi, capability );
    QCOMPARE( list.size(), 1 );
    QCOMPARE( list.at( 0 ).udi(), QString( "/fake/acpi_CPU0" ) );

    capability = Solid::Capability::Unknown;
    list = manager.findDevicesFromQuery( parentUdi, capability,
                                         "blup" );
    QCOMPARE( list.size(), 0 );

}

void SolidTest::slotPropertyChanged( const QMap<QString,int> &changes )
{
    m_changesList << changes;
}

#include "solidtest.moc"

