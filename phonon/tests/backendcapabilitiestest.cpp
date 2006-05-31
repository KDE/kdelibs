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
#include "../videooutputdevice.h"
#include "../videocapturedevice.h"
#include "../visualizationeffect.h"
#include "../audioeffectdescription.h"
#include "../videoeffectdescription.h"
#include "../audiocodec.h"
#include "../videocodec.h"
#include "../containerformat.h"
#include <QStringList>
#include <QSet>

using namespace Phonon;

void BackendCapabilitiesTest::initTestCase()
{
	QVERIFY( BackendCapabilities::self() );
}

void BackendCapabilitiesTest::checkMimeTypes()
{
	QVERIFY( Factory::self()->backend( false ) == 0 );
	QStringList mimeTypes = BackendCapabilities::knownMimeTypes();
	QVERIFY( mimeTypes.size() > 0 ); // a backend that doesn't know any mimetypes is useless
	foreach( QString mimeType, mimeTypes ) {
		qDebug( "%s", qPrintable( mimeType ) );
		QVERIFY( BackendCapabilities::isMimeTypeKnown( mimeType ) );
	}
	QVERIFY( Factory::self()->backend( false ) == 0 ); // the backend should not have been created at this point
	QVERIFY( Factory::self()->backend( true ) != 0 );  // create the backend
	QStringList realMimeTypes = BackendCapabilities::knownMimeTypes(); // this list has to be a subset of the one before
	foreach( QString mimeType, realMimeTypes ) {
		qDebug( "%s", qPrintable( mimeType ) );
		QVERIFY( BackendCapabilities::isMimeTypeKnown( mimeType ) );
		QVERIFY( mimeTypes.contains( mimeType ) );
	}
}

#define VERIFY_TUPLE( classname, availableMethod, indexMethod ) \
QVERIFY( BackendCapabilities::availableMethod().size() >= 0 ); \
for( int i = 0; i < BackendCapabilities::availableMethod().size(); ++i ) \
{ \
	classname device = BackendCapabilities::availableMethod().at( i ); \
	QVERIFY( device.index() >= 0 ); \
	QObject* backend = Factory::self()->backend(); \
	QSet<int> indexes; \
	QMetaObject::invokeMethod( backend, #indexMethod, Qt::DirectConnection, Q_RETURN_ARG( QSet<int>, indexes ) ); \
	QVERIFY( indexes.contains( device.index() ) ); \
	QVERIFY( !device.name().isEmpty() ); \
} do {} while( false )

void BackendCapabilitiesTest::sensibleValues()
{
	//if( BackendCapabilities::supportsVideo() ) create VideoWidget and such - needs UI libs
	VERIFY_TUPLE( AudioOutputDevice, availableAudioOutputDevices, audioOutputDeviceIndexes );
	VERIFY_TUPLE( AudioCaptureDevice, availableAudioCaptureDevices, audioCaptureDeviceIndexes );
	VERIFY_TUPLE( VideoOutputDevice, availableVideoOutputDevices, videoOutputDeviceIndexes );
	VERIFY_TUPLE( VideoCaptureDevice, availableVideoCaptureDevices, videoCaptureDeviceIndexes );
	VERIFY_TUPLE( VisualizationEffect, availableVisualizationEffects, visualizationIndexes );
	VERIFY_TUPLE( AudioEffectDescription, availableAudioEffects, audioEffectIndexes );
	VERIFY_TUPLE( VideoEffectDescription, availableVideoEffects, videoEffectIndexes );
	VERIFY_TUPLE( AudioCodec, availableAudioCodecs, audioCodecIndexes );
	VERIFY_TUPLE( VideoCodec, availableVideoCodecs, videoCodecIndexes );
	VERIFY_TUPLE( ContainerFormat, availableContainerFormats, containerFormatIndexes );
}

void BackendCapabilitiesTest::checkSignals()
{
	QSignalSpy spy( BackendCapabilities::self(), SIGNAL( capabilitiesChanged() ) );
	QCOMPARE( spy.size(), 0 );
	QMetaObject::invokeMethod( Factory::self(), "phononBackendChanged", Qt::DirectConnection );
	QCOMPARE( spy.size(), 1 );
}

void BackendCapabilitiesTest::cleanupTestCase()
{
}

QTEST_KDEMAIN( BackendCapabilitiesTest, NoGUI )
#include "backendcapabilitiestest.moc"
// vim: sw=4 ts=4 noet
