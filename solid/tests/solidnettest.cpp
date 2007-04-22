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

#include <solid/experimental/networkmanager.h>
#include <solid/experimental/networkinterface.h>
#include <solid/experimental/managerbase_p.h>
#include <kdebug.h>

#include <fakenetworkmanager.h>

#ifndef FAKE_NETWORKING_XML
    #error "FAKE_NETWORKING_XML not set. An XML file describing a networking context is required for this test"
#endif

QTEST_KDEMAIN_CORE( SolidNetTest )

void SolidNetTest::initTestCase()
{
    fakeManager = new FakeNetworkManager(0, QStringList(), FAKE_NETWORKING_XML);
    SolidExperimental::ManagerBasePrivate::_k_forcePreloadedBackend("SolidExperimental::Ifaces::NetworkManager", fakeManager);
}

void SolidNetTest::testNetworkInterfaces()
{
    SolidExperimental::NetworkInterfaceList interfaces = SolidExperimental::NetworkManager::networkInterfaces();

    // Verify that the framework reported correctly the interfaces available
    // in the backend.
    QSet<QString> expected_unis, received_unis;

    expected_unis = QSet<QString>::fromList( fakeManager->networkInterfaces() );

    foreach ( SolidExperimental::NetworkInterface iface , interfaces )
    {
        received_unis << iface.uni();
    }

    QCOMPARE( expected_unis, received_unis );
}

void SolidNetTest::testFindNetworkInterface()
{
    QCOMPARE( SolidExperimental::NetworkManager::findNetworkInterface( "/org/kde/solid/fakenet/eth0" ).isValid(), true );
    QCOMPARE( SolidExperimental::NetworkManager::findNetworkInterface( "/org/kde/solid/fakenet/eth1" ).isValid(), true );

    // Note the extra space
    QCOMPARE( SolidExperimental::NetworkManager::findNetworkInterface( "/org/kde/solid/fakenet/eth0 " ).isValid(), false );
    QCOMPARE( SolidExperimental::NetworkManager::findNetworkInterface( "#'({(�" ).isValid(), false );
    QCOMPARE( SolidExperimental::NetworkManager::findNetworkInterface( QString() ).isValid(), false );
}

void SolidNetTest::testManagerBasicFeatures()
{
    QCOMPARE( SolidExperimental::NetworkManager::isNetworkingEnabled(), true );
    QCOMPARE( SolidExperimental::NetworkManager::isWirelessEnabled(), true );

    SolidExperimental::NetworkManager::setNetworkingEnabled( false );

    QCOMPARE( SolidExperimental::NetworkManager::isNetworkingEnabled(), false );
    QCOMPARE( SolidExperimental::NetworkManager::isWirelessEnabled(), false );

    SolidExperimental::NetworkManager::setNetworkingEnabled( true );

    QCOMPARE( SolidExperimental::NetworkManager::isNetworkingEnabled(), true );
    QCOMPARE( SolidExperimental::NetworkManager::isWirelessEnabled(), true );

    SolidExperimental::NetworkManager::setWirelessEnabled( false );

    QCOMPARE( SolidExperimental::NetworkManager::isNetworkingEnabled(), true );
    QCOMPARE( SolidExperimental::NetworkManager::isWirelessEnabled(), false );

    SolidExperimental::NetworkManager::setNetworkingEnabled( false );

    QCOMPARE( SolidExperimental::NetworkManager::isNetworkingEnabled(), false );
    QCOMPARE( SolidExperimental::NetworkManager::isWirelessEnabled(), false );

    SolidExperimental::NetworkManager::setNetworkingEnabled( true );

    QCOMPARE( SolidExperimental::NetworkManager::isNetworkingEnabled(), true );
    QCOMPARE( SolidExperimental::NetworkManager::isWirelessEnabled(), false );

    SolidExperimental::NetworkManager::setWirelessEnabled( true );

    QCOMPARE( SolidExperimental::NetworkManager::isNetworkingEnabled(), true );
    QCOMPARE( SolidExperimental::NetworkManager::isWirelessEnabled(), true );
}

void SolidNetTest::testInterfaceBasicFeatures()
{
    // Retrieve a valid NetworkInterface object
    SolidExperimental::NetworkInterface valid_iface( "/org/kde/solid/fakenet/eth0" );

    QCOMPARE( valid_iface.isValid(), true );


    // A few attempts at creating invalid Device objects
    SolidExperimental::NetworkInterface invalid_iface( "uhoh? doesn't exist, I guess" );
    QCOMPARE( invalid_iface.isValid(), false );
    invalid_iface = SolidExperimental::NetworkManager::findNetworkInterface( QString() );
    QCOMPARE( invalid_iface.isValid(), false );
    invalid_iface = SolidExperimental::NetworkInterface();
    QCOMPARE( invalid_iface.isValid(), false );



    QCOMPARE( valid_iface.uni(), QString( "/org/kde/solid/fakenet/eth0" ) );
    QCOMPARE( invalid_iface.uni(), QString() );


    QCOMPARE( valid_iface.isActive(), true );
    QCOMPARE( valid_iface.type(), SolidExperimental::NetworkInterface::Ieee80211 );
    QCOMPARE( valid_iface.connectionState(), SolidExperimental::NetworkInterface::NeedUserKey );
    QCOMPARE( valid_iface.signalStrength(), 90 );
    QCOMPARE( valid_iface.designSpeed(), 83886080 );
    QCOMPARE( valid_iface.isLinkUp(), true );
    QCOMPARE( valid_iface.capabilities(), SolidExperimental::NetworkInterface::IsManageable
                                         |SolidExperimental::NetworkInterface::SupportsCarrierDetect
                                         |SolidExperimental::NetworkInterface::SupportsWirelessScan );

    QVERIFY( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net1" )!=0 );
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net1" )->isValid(), true );

    QVERIFY( valid_iface.findNetwork( "emldzn" )!=0 );
    QCOMPARE( valid_iface.findNetwork( "emldzn" )->isValid(), false );

    QVERIFY( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth1/net0" )!=0 );
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth1/net0" )->isValid(), false );

    QVERIFY( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net0 " )!=0 );
    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net0" )->isValid(), true );

    QCOMPARE( valid_iface.findNetwork( "/org/kde/solid/fakenet/eth0/net0" )->addressEntries().size(), 1 );
    QCOMPARE( valid_iface.networks().size(), 4 );
}

#include "solidnettest.moc"

