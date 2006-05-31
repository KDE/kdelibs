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

using namespace Phonon;

void MethodTest::checkUiBackendMethods_data() { addColumns();
#include "methods/factory.cpp"
}
//void MethodTest::checkVideoWidgetMethods_data() { addColumns();
//#include "methods/videowidget.cpp"
//}
void MethodTest::checkUiBackendMethods()   { checkMethods( UiFactory::self()->backend() ); }
//void MethodTest::checkVideoWidgetMethods() { checkMethods( UiFactory::self()->createVideoWidget() ); }

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
	QVERIFY( backendObject );
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

QTEST_KDEMAIN( MethodTest, GUI )
#include "methodtest.moc"
// vim: sw=4 ts=4 noet
