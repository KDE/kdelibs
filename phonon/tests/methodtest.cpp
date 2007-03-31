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
#include <phonon/backendinterface.h>
#include <phonon/audiooutputinterface.h>
#include <phonon/addoninterface.h>

using namespace Phonon;

void MethodTest::checkBackendInterface()
{
    QVERIFY(qobject_cast<Phonon::BackendInterface*>(Factory::backend()) != 0);
}

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
void MethodTest::checkAudioDataOutputMethods()   { checkMethods( Factory::createAudioDataOutput() ); }
void MethodTest::checkAudioEffectMethods()       { checkMethods( Factory::createAudioEffect( 1 ) ); }
void MethodTest::checkAudioOutputMethods()       { checkMethods( Factory::createAudioOutput() ); }
void MethodTest::checkAudioPathMethods()         { checkMethods( Factory::createAudioPath() ); }
void MethodTest::checkAvCaptureMethods()         { checkMethods( Factory::createAvCapture() ); }
void MethodTest::checkBackendMethods()           { checkMethods( Factory::backend() ); }
void MethodTest::checkBrightnessControlMethods() { checkMethods( Factory::createBrightnessControl() ); }
void MethodTest::checkByteStreamMethods()        { checkMethods( Factory::createByteStream() ); }
void MethodTest::checkMediaObjectMethods()       { checkMethods( Factory::createMediaObject() ); }
void MethodTest::checkMediaQueueMethods()        { checkMethods( Factory::createMediaQueue() ); }
void MethodTest::checkVideoDataOutputMethods()   { checkMethods( Factory::createVideoDataOutput() ); }
void MethodTest::checkVideoEffectMethods()       { checkMethods( Factory::createVideoEffect( 1 ) ); }
void MethodTest::checkVideoPathMethods()         { checkMethods( Factory::createVideoPath() ); }
void MethodTest::checkVisualizationMethods()     { checkMethods( Factory::createVisualization() ); }
void MethodTest::checkVolumeFaderEffectMethods() { checkMethods( Factory::createVolumeFaderEffect() ); }

void MethodTest::checkByteStreamInterfaces()
{
	QObject* backendObject = Factory::createByteStream();
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
	QVERIFY( qobject_cast<Phonon::ByteStreamInterface*>( backendObject ) != 0 );
	QVERIFY( qobject_cast<Phonon::MediaProducerInterface*>( backendObject ) != 0 );
	QVERIFY( qobject_cast<Phonon::MediaObjectInterface*>( backendObject ) == 0 );
    QVERIFY(qobject_cast<Phonon::AddonInterface *>(backendObject) != 0);
    delete backendObject;
}

void MethodTest::checkMediaObjectInterfaces()
{
	QObject* backendObject = Factory::createMediaObject();
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
	QVERIFY( qobject_cast<Phonon::MediaProducerInterface*>( backendObject ) != 0 );
	QVERIFY( qobject_cast<Phonon::MediaObjectInterface*>( backendObject ) != 0 );
    QVERIFY(qobject_cast<Phonon::AddonInterface *>(backendObject) != 0);
    delete backendObject;
}

void MethodTest::checkAvCapturetInterfaces()
{
	QObject* backendObject = Factory::createAvCapture();
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
	QVERIFY( qobject_cast<Phonon::MediaProducerInterface*>( backendObject ) != 0 );
    delete backendObject;
}

void MethodTest::checkAudioOutputInterface()
{
    QObject *backendObject = Factory::createAudioOutput();
    if (!backendObject)
        QSKIP("The back-end's create method returned 0. No tests possible.", SkipAll);
    QVERIFY(qobject_cast<Phonon::AudioOutputInterface *>(backendObject) != 0);
    delete backendObject;
}

void MethodTest::addColumns()
{
	QTest::addColumn<QByteArray>( "returnType" );
	QTest::addColumn<QByteArray>( "signature" );
	QTest::addColumn<bool>( "optional" );
    QTest::addColumn<bool>("isSignal");
}

void MethodTest::addMethod( const char* returnType, const char* signature, bool optional )
{
	QByteArray name( returnType );
	name += ' ';
	name += signature;
	QTest::newRow( name.constData() ) << QByteArray( returnType ) << QByteArray( signature ) << optional << false;
}

void MethodTest::addSignal(const char *signature)
{
    QTest::newRow(signature) << QByteArray() << QByteArray(signature) << false << true;
}

void MethodTest::checkMethods( QObject* backendObject )
{
	if( !backendObject )
		QSKIP( "The back-end's create method returned 0. No tests possible.", SkipAll );
    const QMetaObject *meta = backendObject->metaObject();

	QFETCH( QByteArray, returnType );
	QFETCH( QByteArray, signature );
	QFETCH( bool, optional );
    QFETCH(bool, isSignal);

    if (isSignal) {
        QVERIFY(meta->indexOfSignal(QMetaObject::normalizedSignature(signature.constData())) != -1);
    } else {
        int index = meta->indexOfMethod(QMetaObject::normalizedSignature(signature.constData()));
        if (index == -1 && optional) {
            QWARN("method is not available - default behaviour will be used instead");
        } else {
            if (index == -1) {
                QFAIL(qPrintable(QString("Method %1 not available!").arg(signature.constData())));
            }
            QVERIFY(index != -1);
            QMetaMethod method = meta->method(index);
            QCOMPARE(method.typeName(), returnType.constData());
        }
    }
}

QTEST_KDEMAIN(MethodTest, NoGUI)
#include "methodtest.moc"
// vim: sw=4 ts=4
