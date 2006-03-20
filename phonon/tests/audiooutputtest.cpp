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

#include "audiooutputtest.h"
#include "../audiooutput.h"
#include <kglobal.h>
#include <QLatin1String>
#include "../phononnamespace.h"
#include <qtest_kde.h>

using namespace Phonon;

void AudioOutputTest::initTestCase()
{
}

void AudioOutputTest::checkName()
{
	AudioOutput ao( this );
	QCOMPARE( ao.name(), QLatin1String( "qttest" ) );
	QString n( "lsdf" );
	ao.setName( n );
	QCOMPARE( ao.name(), n );
}

void AudioOutputTest::checkVolume()
{
	AudioOutput ao( this );
	QSignalSpy volumeSignalSpy( &ao, SIGNAL( volumeChanged( float ) ) );
	float v = 1.0;
	QCOMPARE( ao.volume(), v );
	QCOMPARE( volumeSignalSpy.size(), 0 );
	for( v = 0.0; v <= 10.0; v += 0.01 )
	{
		ao.setVolume( v );
		QCOMPARE( ao.volume(), v );
		QCOMPARE( volumeSignalSpy.size(), 1 );
		QCOMPARE( qvariant_cast<float>( volumeSignalSpy.takeFirst().at( 0 ) ), v );
	}
}

void AudioOutputTest::checkCategory()
{
	AudioOutput ao( this );
	ao.setCategory( Phonon::UnspecifiedCategory );
	QCOMPARE( ao.category(), Phonon::UnspecifiedCategory );
	QCOMPARE( Phonon::categoryToString( ao.category() ), QLatin1String( "Unspecified" ) );
	ao.setCategory( Phonon::NotificationCategory );
	QCOMPARE( ao.category(), Phonon::NotificationCategory );
	QCOMPARE( Phonon::categoryToString( ao.category() ), QLatin1String( "Notification" ) );
	ao.setCategory( Phonon::MusicCategory );
	QCOMPARE( ao.category(), Phonon::MusicCategory );
	QCOMPARE( Phonon::categoryToString( ao.category() ), QLatin1String( "Music" ) );
	ao.setCategory( Phonon::VideoCategory );
	QCOMPARE( ao.category(), Phonon::VideoCategory );
	QCOMPARE( Phonon::categoryToString( ao.category() ), QLatin1String( "Video" ) );
	ao.setCategory( Phonon::CommunicationCategory );
	QCOMPARE( ao.category(), Phonon::CommunicationCategory );
	QCOMPARE( Phonon::categoryToString( ao.category() ), QLatin1String( "Communication" ) );
}

void AudioOutputTest::cleanupTestCase()
{
}

QTEST_KDEMAIN( AudioOutputTest, NoGUI )
#include "audiooutputtest.moc"
// vim: sw=4 ts=4 noet
