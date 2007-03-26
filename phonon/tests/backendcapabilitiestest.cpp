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
#include "../objectdescription.h"
#include <QStringList>
#include <QSet>
#include "../backendinterface.h"
#include "loadfakebackend.h"
#include <kservice.h>
#include <kservicetypetrader.h>

using namespace Phonon;

void BackendCapabilitiesTest::initTestCase()
{
    QVERIFY(BackendCapabilities::notifier());
}

void BackendCapabilitiesTest::checkMimeTypes()
{
	QVERIFY( Factory::backend( false ) == 0 );
#ifdef USE_FAKE_BACKEND
    QStringList mimeTypes;
    const KService::List offers = KServiceTypeTrader::self()->query("PhononBackend",
            "Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1 "
            "and Library == 'phonon_fake' and [X-KDE-PhononBackendInfo-Version] == '0.1'");
    if (!offers.isEmpty()) {
        mimeTypes = offers.first()->serviceTypes();
        mimeTypes.removeAll("PhononBackend");
    }
#else
	QStringList mimeTypes = BackendCapabilities::knownMimeTypes();
#endif
	QVERIFY( mimeTypes.size() > 0 ); // a backend that doesn't know any mimetypes is useless
	foreach( QString mimeType, mimeTypes ) {
		qDebug( "%s", qPrintable( mimeType ) );
		QVERIFY( BackendCapabilities::isMimeTypeKnown( mimeType ) );
	}
	QVERIFY( Factory::backend( false ) == 0 ); // the backend should not have been created at this point
#ifdef USE_FAKE_BACKEND
    Phonon::loadFakeBackend();
#endif
	QVERIFY( Factory::backend( true ) != 0 );  // create the backend
	QStringList realMimeTypes = BackendCapabilities::knownMimeTypes(); // this list has to be a subset of the one before
	foreach( QString mimeType, realMimeTypes ) {
		qDebug( "%s", qPrintable( mimeType ) );
		QVERIFY( BackendCapabilities::isMimeTypeKnown( mimeType ) );
		QVERIFY( mimeTypes.contains( mimeType ) );
	}
}

#define VERIFY_TUPLE(T) \
QVERIFY(BackendCapabilities::available##T##s().size() >= 0); \
do { \
    for (int i = 0; i < BackendCapabilities::available##T##s().size(); ++i) { \
        ObjectDescription<T ## Type> device = BackendCapabilities::available##T##s().at(i); \
        QVERIFY(device.index() >= 0); \
        QSet<int> indexes = iface->objectDescriptionIndexes(Phonon::T##Type);\
        QVERIFY(indexes.contains( device.index())); \
        QVERIFY(!device.name().isEmpty()); \
    } \
} while(false)

void BackendCapabilitiesTest::sensibleValues()
{
    BackendInterface *iface = qobject_cast<BackendInterface*>(Factory::backend());
	//if( BackendCapabilities::supportsVideo() ) create VideoWidget and such - needs UI libs
	VERIFY_TUPLE( AudioOutputDevice );
	VERIFY_TUPLE( AudioCaptureDevice );
	VERIFY_TUPLE( VideoOutputDevice );
	VERIFY_TUPLE( VideoCaptureDevice );
	VERIFY_TUPLE( Visualization );
	VERIFY_TUPLE( AudioEffect );
	VERIFY_TUPLE( VideoEffect );
	VERIFY_TUPLE( AudioCodec );
	VERIFY_TUPLE( VideoCodec );
	VERIFY_TUPLE( ContainerFormat );
}

void BackendCapabilitiesTest::checkSignals()
{
    QSignalSpy spy(BackendCapabilities::notifier(), SIGNAL(capabilitiesChanged()));
	QCOMPARE( spy.size(), 0 );
    QMetaObject::invokeMethod(Factory::sender(), "phononBackendChanged", Qt::DirectConnection);
	QCOMPARE( spy.size(), 1 );
}

void BackendCapabilitiesTest::cleanupTestCase()
{
}

QTEST_KDEMAIN( BackendCapabilitiesTest, NoGUI )
#include "backendcapabilitiestest.moc"
// vim: sw=4 ts=4
