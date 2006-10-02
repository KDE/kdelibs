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

#include "solidnettest.h"

#include <qtest_kde.h>

#include <solid/networkmanager.h>
#include <solid/networkinterface.h>
#include <kdebug.h>

#include <fakenetworkmanager.h>

#ifndef FAKE_NETWORKING_XML
    #error "FAKE_NETWORKING_XML not set. An XML file describing a networking context is required for this test"
#endif

QTEST_KDEMAIN( SolidNetTest, NoGUI )

void SolidNetTest::initTestCase()
{
    fakeManager = new FakeNetworkManager(0, QStringList(), FAKE_NETWORKING_XML);
    Solid::NetworkManager::selfForceBackend( fakeManager );
}

void SolidNetTest::testNetworkInterfaces()
{
    Solid::NetworkManager &manager = Solid::NetworkManager::self();

    Solid::NetworkInterfaceList interfaces = manager.networkInterfaces();

    // Verify that the framework reported correctly the interfaces available
    // in the backend.
    QSet<QString> expected_unis, received_unis;

    expected_unis = QSet<QString>::fromList( fakeManager->networkInterfaces() );

    foreach ( Solid::NetworkInterface iface , interfaces )
    {
        received_unis << iface.uni();
    }

    QCOMPARE( expected_unis, received_unis );
}

void SolidNetTest::testFindNetworkInterface()
{
    Solid::NetworkManager &manager = Solid::NetworkManager::self();

    QCOMPARE( manager.findNetworkInterface( "/org/kde/solid/fakenet/eth0" ).isValid(), true );
    QCOMPARE( manager.findNetworkInterface( "/org/kde/solid/fakenet/eth1" ).isValid(), true );

    // Note the extra space
    QCOMPARE( manager.findNetworkInterface( "/org/kde/solid/fakenet/eth0 " ).isValid(), false );
    QCOMPARE( manager.findNetworkInterface( "#'({(�" ).isValid(), false );
    QCOMPARE( manager.findNetworkInterface( QString() ).isValid(), false );
}

void SolidNetTest::testManagerBasicFeatures()
{
    Solid::NetworkManager &manager = Solid::NetworkManager::self();

    QCOMPARE( manager.isNetworkingEnabled(), true );
    QCOMPARE( manager.isWirelessEnabled(), true );

    manager.setNetworkingEnabled( false );

    QCOMPARE( manager.isNetworkingEnabled(), false );
    QCOMPARE( manager.isWirelessEnabled(), false );

    manager.setNetworkingEnabled( true );

    QCOMPARE( manager.isNetworkingEnabled(), true );
    QCOMPARE( manager.isWirelessEnabled(), true );

    manager.setWirelessEnabled( false );

    QCOMPARE( manager.isNetworkingEnabled(), true );
    QCOMPARE( manager.isWirelessEnabled(), false );

    manager.setNetworkingEnabled( false );

    QCOMPARE( manager.isNetworkingEnabled(), false );
    QCOMPARE( manager.isWirelessEnabled(), false );

    manager.setNetworkingEnabled( true );

    QCOMPARE( manager.isNetworkingEnabled(), true );
    QCOMPARE( manager.isWirelessEnabled(), false );

    manager.setWirelessEnabled( true );

    QCOMPARE( manager.isNetworkingEnabled(), true );
    QCOMPARE( manager.isWirelessEnabled(), true );
}

void SolidNetTest::testInterfaceBasicFeatures()
{
    Solid::NetworkManager &manager = Solid::NetworkManager::self();

    // Retrieve a valid NetworkInterface object
    Solid::NetworkInterface valid_iface( "/org/kde/solid/fakenet/eth0" );

    QCOMPARE( valid_iface.isValid(), true );


    // A few attempts at creating invalid Device objects
    Solid::NetworkInterface invalid_iface( "uhoh? doesn't exist, I guess" );
    QCOMPARE( invalid_iface.isValid(), false );
    invalid_iface = manager.findNetworkInterface( QString() );
    QCOMPARE( invalid_iface.isValid(), false );
    invalid_iface = Solid::NetworkInterface();
    QCOMPARE( invalid_iface.isValid(), false );



    QCOMPARE( valid_iface.uni(), QString( "/org/kde/solid/fakenet/eth0" ) );
    QCOMPARE( invalid_iface.uni(), QString() );


    QCOMPARE( valid_iface.isActive(), true );
    QCOMPARE( valid_iface.type(), Solid::NetworkInterface::Ieee80211 );
    QCOMPARE( valid_iface.connectionState(), Solid::NetworkInterface::NeedUserKey );
    QCOMPARE( valid_iface.signalStrength(), 90 );
    QCOMPARE( valid_iface.designSpeed(), 83886080 );
    QCOMPARE( valid_iface.isLinkUp(), true );
    QCOMPARE( valid_iface.capabilities(), Solid::NetworkInterface::IsManageable
                                         |Solid::NetworkInterface::SupportsCarrierDetect
                                         |Solid::NetworkInterface::SupportsWirelessScan );

    QVERIFY( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net1" )!=0 );
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net1" )->isValid(), true );

    QVERIFY( valid_iface.findNetwork( "emldzn" )!=0 );
    QCOMPARE( valid_iface.findNetwork( "emldzn" )->isValid(), false );

    QVERIFY( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth1/net0" )!=0 );
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth1/net0" )->isValid(), false );

    QVERIFY( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net0 " )!=0 );
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net0" )->isValid(), true );
    
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net0" )->ipV4Addresses().size(), 1 );
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net0" )->ipV6Addresses().size(), 2 );

    QCOMPARE( valid_iface.networks().size(), 4 );
}

#include "solidnettest.moc"

