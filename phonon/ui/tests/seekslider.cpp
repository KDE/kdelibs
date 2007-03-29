/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#include "seekslider.h"
#include "../seekslider.h"
#include "../../mediaobject.h"
#include <QSlider>
#include <kurl.h>
#include <cstdlib>
#include <QLabel>
#include "../../tests/loadfakebackend.h"

using namespace Phonon;

void SeekSliderTest::initTestCase()
{
        Phonon::loadFakeBackend();
	ss = new SeekSlider;
	QVERIFY( ss != 0 );
	qslider = ss->findChild<QSlider*>();
	qlabel = ss->findChild<QLabel*>();
	QVERIFY( qslider != 0 );
	QVERIFY( qlabel != 0 );
	media = new MediaObject( this );
}

void SeekSliderTest::testEnabled()
{
	QVERIFY( !qslider->isEnabled() );
	ss->setMediaProducer( 0 );
	QVERIFY( !qslider->isEnabled() );
}

void SeekSliderTest::setMedia()
{
	QVERIFY( media->state() == Phonon::LoadingState );
	KUrl url( getenv( "PHONON_TESTURL" ) );
	if( !url.isValid() )
		QFAIL( "You need to set PHONON_TESTURL to a valid URL" );
	media->setUrl( url );
	ss->setMediaProducer( media );
	QVERIFY( !qslider->isEnabled() );
}

void SeekSliderTest::playMedia()
{
	media->play();
	if( media->state() != Phonon::PlayingState )
		QVERIFY( !qslider->isEnabled() );
	else
		QVERIFY( qslider->isEnabled() );
}

void SeekSliderTest::seekWithSlider()
{
	// click on the slider to seek
}

void SeekSliderTest::cleanupTestCase()
{
	delete media;
	qslider = 0;
	delete ss;
}

QTEST_KDEMAIN( SeekSliderTest, GUI )
#include "seekslider.moc"
