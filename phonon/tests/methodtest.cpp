/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "methodtest.h"
#include "../factory.h"
#include <qtest_kde.h>
#include <phonon/bytestreaminterface.h>
#include <phonon/mediaproducerinterface.h>
#include <phonon/mediaobjectinterface.h>

using namespace Phonon;

void MethodTest::checkAudioDataOutputMethods_data() { addColumns();
#include "methods/abstractaudiooutput.cpp"
#include "methods/audiodataoutput.cpp"
}
void MethodTest::checkAudioEffectMethods_data() { addColumns();
#include "methods/audioeffect.cpp"
}
void MethodTest::checkAudioOutputMethods_data() { addColumns();
#include "methods/abstractaudiooutput.cpp"
#include "methods/audiooutput.cpp"
}
void MethodTest::checkAudioPathMethods_data() { addColumns();
#include "methods/audiopath.cpp"
}
void MethodTest::checkAvCaptureMethods_data() { addColumns();
#include "methods/abstractmediaproducer.cpp"
#include "methods/avcapture.cpp"
}
void MethodTest::checkBackendMethods_data() { addColumns();
#include "methods/factory.cpp"
#include "methods/objectdescription.cpp"
#include "methods/backendcapabilities.cpp"
addMethod( "bool", "supportsFourcc(quint32)" );
}
void MethodTest::checkBrightnessControlMethods_data() { addColumns();
#include "methods/brightnesscontrol.cpp"
}
void MethodTest::checkByteStreamMethods_data() { addColumns();
#include "methods/abstractmediaproducer.cpp"
#include "methods/bytestream.cpp"
}
void MethodTest::checkMediaObjectMethods_data() { addColumns();
#include "methods/abstractmediaproducer.cpp"
#include "methods/mediaobject.cpp"
}
void MethodTest::checkMediaQueueMethods_data() { addColumns();
#include "methods/mediaqueue.cpp"
}
void MethodTest::checkVideoDataOutputMethods_data() { addColumns();
#include "methods/abstractvideooutput.cpp"
#include "methods/videodataoutput.cpp"
}
void MethodTest::checkVideoEffectMethods_data() { addColumns();
#include "methods/videoeffect.cpp"
}
void MethodTest::checkVideoPathMethods_data() { addColumns();
#include "methods/videopath.cpp"
}
void MethodTest::checkVisualizationMethods_data() { addColumns();
#include "methods/visualization.cpp"
}
void MethodTest::checkVolumeFaderEffectMethods_data() { addColumns();
#include "methods/volumefadereffect.cpp"
}
void MethodTest::checkAudioDataOutputMethods()   { checkMethods( Factory::self()->createAudioDataOutput() ); }
void MethodTest::checkAudioEffectMethods()       { checkMethods( Factory::self()->createAudioEffect( 1 ) ); }
void MethodTest::checkAudioOutputMethods()       { checkMethods( Factory::self()->createAudioOutput() ); }
void MethodTest::checkAudioPathMethods()         { checkMethods( Factory::self()->createAudioPath() ); }
void MethodTest::checkAvCaptureMethods()         { checkMethods( Factory::self()->createAvCapture() ); }
void MethodTest::checkBackendMethods()           { checkMethods( Factory::self()->backend() ); }
void MethodTest::checkBrightnessControlMethods() { checkMethods( Factory::self()->createBrightnessControl() ); }
void MethodTest::checkByteStreamMethods()        { checkMethods( Factory::self()->createByteStream() ); }
void MethodTest::checkMediaObjectMethods()       { checkMethods( Factory::self()->createMediaObject() ); }
void MethodTest::checkMediaQueueMethods()        { checkMethods( Factory::self()->createMediaQueue() ); }
void MethodTest::checkVideoDataOutputMethods()   { checkMethods( Factory::self()->createVideoDataOutput() ); }
void MethodTest::checkVideoEffectMethods()       { checkMethods( Factory::self()->createVideoEffect( 1 ) ); }
void MethodTest::checkVideoPathMethods()         { checkMethods( Factory::self()->createVideoPath() ); }
void MethodTest::checkVisualizationMethods()     { checkMethods( Factory::self()->createVisualization() ); }
void MethodTest::checkVolumeFaderEffectMethods() { checkMethods( Factory::self()->createVolumeFaderEffect() ); }

void MethodTest::checkByteStreamInterfaces()
{
	QObject* backendObject = Factory::self()->createByteStream();
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
	QVERIFY( qobject_cast<Phonon::ByteStreamInterface*>( backendObject ) != 0 );
	QVERIFY( qobject_cast<Phonon::MediaProducerInterface*>( backendObject ) != 0 );
}

void MethodTest::checkMediaObjectInterfaces()
{
	QObject* backendObject = Factory::self()->createMediaObject();
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
	QVERIFY( qobject_cast<Phonon::MediaProducerInterface*>( backendObject ) != 0 );
	QVERIFY( qobject_cast<Phonon::MediaObjectInterface*>( backendObject ) != 0 );
}

void MethodTest::checkAvCapturetInterfaces()
{
	QObject* backendObject = Factory::self()->createAvCapture();
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
	QVERIFY( qobject_cast<Phonon::MediaProducerInterface*>( backendObject ) != 0 );
}

void MethodTest::addColumns()
{
	QTest::addColumn<QByteArray>( "returnType" );
	QTest::addColumn<QByteArray>( "signature" );
	QTest::addColumn<bool>( "optional" );
}

void MethodTest::addMethod( const char* returnType, const char* signature, bool optional )
{
	QByteArray name( returnType );
	name += ' ';
	name += signature;
	QTest::newRow( name.constData() ) << QByteArray( returnType ) << QByteArray( signature ) << optional;
}

void MethodTest::checkMethods( QObject* backendObject )
{
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
	meta = backendObject->metaObject();

	QFETCH( QByteArray, returnType );
	QFETCH( QByteArray, signature );
	QFETCH( bool, optional );

	int index = meta->indexOfMethod( QMetaObject::normalizedSignature( signature.constData() ) );
	if( index == -1 && optional )
	{
		QWARN( "method is not available - default behaviour will be used instead" );
	}
	else
	{
		QVERIFY( index != -1 );
		QMetaMethod method = meta->method( index );
		QCOMPARE( method.typeName(), returnType.constData() );
	}
}

QTEST_KDEMAIN( MethodTest, NoGUI )
#include "methodtest.moc"
// vim: sw=4 ts=4
