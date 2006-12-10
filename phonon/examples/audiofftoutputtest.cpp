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

#include "audiofftoutputtest.h"
#include "../audiofftoutput.h"
#include <qtest_kde.h>

using namespace Phonon;

void AudioFftOutputTest::initTestCase()
{
	AudioFftOutput test( this );
}

void AudioFftOutputTest::testBandwidth()
{
	AudioFftOutput test( this );
	int b = test.setBandwidth( 100 );
	QVERIFY( b <= 100 );
	QVERIFY( b >= 50 );
	QVERIFY( b == test.bandwidth() );
}

void AudioFftOutputTest::testRate()
{
	AudioFftOutput test( this );
	int rate = 50;
	test.setRate( rate );
	QCOMPARE( test.rate(), rate );
	rate = 30;
	test.setRate( rate );
	QCOMPARE( test.rate(), rate );
}

void AudioFftOutputTest::testIndependenceOfRateAndBandwidth()
{
	AudioFftOutput test( this );
	int b = test.setBandwidth( 100 );
	QVERIFY( b <= 100 );
	QVERIFY( b >= 50 );
	QVERIFY( b == test.bandwidth() );
	int rate = 50;
	test.setRate( rate );
	QCOMPARE( test.rate(), rate );
	QVERIFY( b == test.bandwidth() );
	b = test.setBandwidth( 150 );
	QVERIFY( b <= 150 );
	QVERIFY( b >= 75 );
	QVERIFY( b == test.bandwidth() );
	QCOMPARE( test.rate(), rate );
}

void AudioFftOutputTest::cleanupTestCase()
{
}

QTEST_KDEMAIN( AudioFftOutputTest, NoGUI )
#include "audiofftoutputtest.moc"
// vim: sw=4 ts=4
