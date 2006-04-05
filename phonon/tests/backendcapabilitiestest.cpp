/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include <qtest_kde.h>
#include "backendcapabilitiestest.h"
#include "../backendcapabilities.h"
#include "../factory.h"
#include "../audiooutputdevice.h"
#include "../audiocapturedevice.h"
#include "../videocapturedevice.h"
#include "../audioeffectdescription.h"
#include "../videoeffectdescription.h"

using namespace Phonon;

void BackendCapabilitiesTest::initTestCase()
{
	QVERIFY( BackendCapabilities::self() );
}

void BackendCapabilitiesTest::sensibleValues()
{
	//if( BackendCapabilities::supportsVideo() ) create VideoWidget and such - needs UI libs
	QVERIFY( BackendCapabilities::knownMimeTypes().size() > 0 ); // a backend that doesn't know any mimetypes is useless
	QVERIFY( BackendCapabilities::availableAudioOutputDevices().size() >= 0 );
	for( int i = 0; i < BackendCapabilities::availableAudioOutputDevices().size(); ++i )
	{
		QVERIFY( BackendCapabilities::availableAudioOutputDevices().at( i ).index() == i+1 );
		QVERIFY( !BackendCapabilities::availableAudioOutputDevices().at( i ).name().isEmpty() );
	}
	QVERIFY( BackendCapabilities::availableAudioCaptureDevices().size() >= 0 );
	for( int i = 0; i < BackendCapabilities::availableAudioCaptureDevices().size(); ++i )
	{
		QVERIFY( BackendCapabilities::availableAudioCaptureDevices().at( i ).index() == i+1 );
		QVERIFY( !BackendCapabilities::availableAudioCaptureDevices().at( i ).name().isEmpty() );
	}
	QVERIFY( BackendCapabilities::availableVideoCaptureDevices().size() >= 0 );
	for( int i = 0; i < BackendCapabilities::availableVideoCaptureDevices().size(); ++i )
	{
		QVERIFY( BackendCapabilities::availableVideoCaptureDevices().at( i ).index() == i+1 );
		QVERIFY( !BackendCapabilities::availableVideoCaptureDevices().at( i ).name().isEmpty() );
	}
	QVERIFY( BackendCapabilities::availableAudioEffects().size() >= 0 );
	for( int i = 0; i < BackendCapabilities::availableAudioEffects().size(); ++i )
	{
		QVERIFY( BackendCapabilities::availableAudioEffects().at( i ).index() == i+1 );
		QVERIFY( !BackendCapabilities::availableAudioEffects().at( i ).name().isEmpty() );
	}
	QVERIFY( BackendCapabilities::availableVideoEffects().size() >= 0 );
	for( int i = 0; i < BackendCapabilities::availableVideoEffects().size(); ++i )
	{
		QVERIFY( BackendCapabilities::availableVideoEffects().at( i ).index() == i+1 );
		QVERIFY( !BackendCapabilities::availableVideoEffects().at( i ).name().isEmpty() );
	}
}

void BackendCapabilitiesTest::checkSignals()
{
	QSignalSpy spy( BackendCapabilities::self(), SIGNAL( capabilitiesChanged() ) );
	QCOMPARE( spy.size(), 0 );
	Factory::self()->phononBackendChanged();
	QCOMPARE( spy.size(), 1 );
}

void BackendCapabilitiesTest::cleanupTestCase()
{
}

QTEST_KDEMAIN( BackendCapabilitiesTest, NoGUI )
#include "backendcapabilitiestest.moc"
// vim: sw=4 ts=4 noet
