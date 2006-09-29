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

#include "solidnettest.moc"

