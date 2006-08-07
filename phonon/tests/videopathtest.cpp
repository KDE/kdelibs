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

#include "videopathtest.h"
#include <cstdlib>
#include <qtest_kde.h>
#include <QTime>
#include <QtDebug>

using namespace Phonon;

Q_DECLARE_METATYPE( Phonon::State )

void VideoPathTest::initTestCase()
{
	qRegisterMetaType<Phonon::State>( "Phonon::State" );

	m_url.setUrl( getenv( "PHONON_TESTURL" ) );
	if( !m_url.isValid() )
		QFAIL( "You need to set PHONON_TESTURL to a valid URL" );

	m_media = new MediaObject( this );
	m_path = new VideoPath( this );
	m_output = new VideoOutput( this );

	m_media->setUrl( m_url );
}

void VideoPathTest::checkForDefaults()
{
	QCOMPARE( m_path->effects().size(), 0 );
	QCOMPARE( m_path->outputs().size(), 0 );
}

void VideoPathTest::addOutputs()
{
	//this is the first output, this has to work:
	QCOMPARE( m_path->addOutput( m_output ), true );
	QCOMPARE( m_path->outputs().size(), 1 );
	QVERIFY( m_path->outputs().contains( m_output ) );
	VideoOutput* o2 = new VideoOutput( this );
	if( m_path->addOutput( o2 ) )
	{
		QCOMPARE( m_path->outputs().size(), 2 );
		QVERIFY( m_path->outputs().contains( m_output ) );
		QVERIFY( m_path->outputs().contains( o2 ) );

		QCOMPARE( m_path->removeOutput( o2 ), true );
		QCOMPARE( m_path->outputs().size(), 1 );
		QVERIFY( m_path->outputs().contains( m_output ) );
		QVERIFY( !m_path->outputs().contains( o2 ) );

		QCOMPARE( m_path->addOutput( o2 ), true );
		QCOMPARE( m_path->outputs().size(), 2 );
		QVERIFY( m_path->outputs().contains( m_output ) );
		QVERIFY( m_path->outputs().contains( o2 ) );

		delete o2;
		QCOMPARE( m_path->outputs().size(), 1 );
		QVERIFY( m_path->outputs().contains( m_output ) );
		QVERIFY( !m_path->outputs().contains( o2 ) );
	}
	else
	{
		QWARN( "VideoPath not able to use more than one VideoOutput" );
		delete o2;
	}
}

void VideoPathTest::addEffects()
{
	QWARN( "VideoEffects not implemented" );
}

void VideoPathTest::cleanupTestCase()
{
	delete m_output;
	delete m_path;
	delete m_media;
}

QTEST_KDEMAIN( VideoPathTest, NoGUI )
#include "videopathtest.moc"
// vim: sw=4 ts=4 noet
