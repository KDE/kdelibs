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

#include "bytestreamtest.h"
#include "qtesthelper.h"
#include <cstdlib>
#include <qtest_kde.h>
#include <QTime>
#include <QtDebug>
#include <phonon/videopath.h>
#include <kio/job.h>
#include <kio/filejob.h>
#include <kprotocolmanager.h>

using namespace Phonon;

Q_DECLARE_METATYPE( Phonon::State )

static qint64 castQVariantToInt64( const QVariant& variant )
{
	return *reinterpret_cast<const qint64*>( variant.constData() );
}

static qint32 castQVariantToInt32( const QVariant& variant )
{
	return *reinterpret_cast<const qint32*>( variant.constData() );
}

void ByteStreamTest::startPlayback(Phonon::State currentState)
{
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), currentState);
	m_media->play();
	QCOMPARE( m_stateChangedSignalSpy->count(), 1 );
	QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
	Phonon::State newstate = qvariant_cast<Phonon::State>( args.at( 0 ) );
	Phonon::State oldstate = qvariant_cast<Phonon::State>( args.at( 1 ) );
	QCOMPARE( oldstate, currentState);
	QCOMPARE( newstate, m_media->state() );
	if( newstate == Phonon::BufferingState )
	{
		QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
		while( m_stateChangedSignalSpy->count() == 0 )
			QCoreApplication::processEvents();
		QCOMPARE( m_stateChangedSignalSpy->count(), 1 );

		args = m_stateChangedSignalSpy->takeFirst();
		newstate = qvariant_cast<Phonon::State>( args.at( 0 ) );
		oldstate = qvariant_cast<Phonon::State>( args.at( 1 ) );
		QCOMPARE( oldstate, Phonon::BufferingState );
	}
	QCOMPARE( newstate, Phonon::PlayingState );
	QCOMPARE( m_media->state(), Phonon::PlayingState );
}

void ByteStreamTest::stopPlayback( Phonon::State currentState )
{
	m_media->stop();
    if (m_stateChangedSignalSpy->count() == 0) {
        QCOMPARE(m_media->state(), currentState);
        while (m_stateChangedSignalSpy->count() == 0) {
            QCoreApplication::processEvents();
        }
    }
	QCOMPARE( m_stateChangedSignalSpy->count(), 1 );
	QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
	Phonon::State newstate = qvariant_cast<Phonon::State>( args.at( 0 ) );
	Phonon::State oldstate = qvariant_cast<Phonon::State>( args.at( 1 ) );
	QCOMPARE( oldstate, currentState );
	QCOMPARE( newstate, Phonon::StoppedState );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
}

void ByteStreamTest::pausePlayback( Phonon::State currentState )
{
	m_media->pause();
    if (m_stateChangedSignalSpy->count() == 0) {
        QCOMPARE(m_media->state(), currentState);
        while (m_stateChangedSignalSpy->count() == 0) {
            QCoreApplication::processEvents();
        }
    }
	QCOMPARE( m_stateChangedSignalSpy->count(), 1 );
	QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
	Phonon::State newstate = qvariant_cast<Phonon::State>( args.at( 0 ) );
	Phonon::State oldstate = qvariant_cast<Phonon::State>( args.at( 1 ) );
	QCOMPARE( oldstate, currentState );
	QCOMPARE( newstate, Phonon::PausedState );
	QCOMPARE( m_media->state(), Phonon::PausedState );
}

void ByteStreamTest::initTestCase()
{
	qRegisterMetaType<Phonon::State>( "Phonon::State" );
	qRegisterMetaType<qint32>( "qint32" );
	qRegisterMetaType<qint64>( "qint64" );

    m_endOfDataSent = false;
    m_reading = false;
    m_seeking = false;

	m_url.setUrl( getenv( "PHONON_TESTURL" ) );
	if( !m_url.isValid() )
		QFAIL( "You need to set PHONON_TESTURL to a valid URL" );

	initByteStream();
}

void ByteStreamTest::initByteStream()
{
	delete m_media;
	delete m_stateChangedSignalSpy;
	m_media = new ByteStream( this );
	m_stateChangedSignalSpy = new QSignalSpy( m_media, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ) );
	QVERIFY( m_stateChangedSignalSpy->isValid() );
	m_stateChangedSignalSpy->clear();
}

void ByteStreamTest::kioSeekDone(KIO::Job*, KIO::filesize_t)
{
    m_seeking = false;
    m_endOfDataSent = false;
    m_reading = true;
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>(m_job);
    QVERIFY(filejob);
    filejob->read(32768);
}

void ByteStreamTest::needData()
{
    if (m_seeking) {
        return;
    }
    if (m_job && m_job->isSuspended()) {
        m_job->resume();
    }
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>(m_job);
    if (filejob) {
        filejob->read(32768);
    }
}

void ByteStreamTest::enoughData()
{
    if (!m_seeking && m_job && !m_job->isSuspended()) {
        m_job->suspend();
    }
    m_reading = false;
}

void ByteStreamTest::seekStream(qint64 offset)
{
    KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>(m_job);
    QVERIFY(filejob);
    m_seeking = true;
    m_reading = false;

    if (m_job->isSuspended()) {
        m_job->resume();
    }
    filejob->seek(offset);
}

void ByteStreamTest::kioJobOpen(KIO::Job*)
{
    m_endOfDataSent = false;
    KIO::FileJob *filejob = static_cast<KIO::FileJob*>(m_job);
    QVERIFY(filejob);
    m_media->setStreamSize(filejob->size());
    m_reading = true;
    filejob->read(32768);
}

void ByteStreamTest::kioTotalSize(KJob*,qulonglong size)
{
	m_media->setStreamSize( size );
}

void ByteStreamTest::kioData(KIO::Job*,const QByteArray& data)
{
    if (m_seeking) {
        m_reading = false;
        return;
    }
    if (data.isEmpty()) {
        m_reading = false;
        if (!m_endOfDataSent) {
            m_endOfDataSent = true;
            m_media->endOfData();
        }
        return;
    }
    m_media->writeData(data);
    if (m_reading) {
        KIO::FileJob *filejob = qobject_cast<KIO::FileJob*>(m_job);
        QVERIFY(filejob);
        filejob->read(32768);
    }
}

void ByteStreamTest::kioResult(KJob*)
{
    m_job = 0; // the job kills itself
    m_endOfDataSent = true;
    m_media->endOfData();
    m_reading = false;
}

void ByteStreamTest::setMedia()
{
	QSignalSpy lengthSignalSpy( m_media, SIGNAL( length( qint64 ) ) );
	QCOMPARE( m_media->state(), Phonon::LoadingState );
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );

	if( m_job )
	{
		disconnect( m_job, SIGNAL(data(KIO::Job*,const QByteArray&)),
				this, SLOT(kioData(KIO::Job*,const QByteArray&)) );
		disconnect( m_job, SIGNAL(result(KJob*)),
				this, SLOT(kioResult(KJob*)) );
		disconnect( m_job, SIGNAL(totalSize(KJob*, qulonglong)),
				this, SLOT(kioTotalSize(KJob*,qulonglong)) );
		m_job->kill();
	}

    if (KProtocolManager::supportsOpening(m_url)) {
        m_job = KIO::open(m_url, 1 /*ReadOnly*/);
        QVERIFY(m_job);
        m_media->setStreamSeekable(true);
        QObject::connect(m_job, SIGNAL(open(KIO::Job*)),
                this, SLOT(kioJobOpen(KIO::Job*)));
        QObject::connect(m_job, SIGNAL(position(KIO::Job*, KIO::filesize_t)),
                this, SLOT(kioSeekDone(KIO::Job*, KIO::filesize_t)));
    } else {
        m_job = KIO::get(m_url, false, false);
        m_media->setStreamSeekable(false);
        connect(m_job, SIGNAL(totalSize(KJob*, qulonglong)),
                this, SLOT(kioTotalSize(KJob*,qulonglong)));
    }
    m_endOfDataSent = false;

	m_job->addMetaData( "UserAgent", QLatin1String( "KDE Phonon" ) );
	connect( m_job, SIGNAL(data(KIO::Job*,const QByteArray&)),
			this, SLOT(kioData(KIO::Job*,const QByteArray&)) );
	connect( m_job, SIGNAL(result(KJob*)),
			this, SLOT(kioResult(KJob*)) );

    connect( m_media, SIGNAL(needData()), SLOT(needData()));
    connect( m_media, SIGNAL(enoughData()), SLOT(enoughData()));
    connect( m_media, SIGNAL(seekStream(qint64)), SLOT(seekStream(qint64)));

	int emits = m_stateChangedSignalSpy->count();
	Phonon::State s = m_media->state();
	if( s == Phonon::LoadingState )
	{
		// still in LoadingState, there should be no state change
		QCOMPARE( emits, 0 );
		QTime startTime;
		startTime.start();
		while( m_stateChangedSignalSpy->count() == 0 )
		{
			QCoreApplication::processEvents();
			QVERIFY( startTime.elapsed() < 30 * 1000 ); // give it 30 seconds to leave LoadingState
		}
		emits = m_stateChangedSignalSpy->count();
		s = m_media->state();
	}
	if( s != Phonon::LoadingState )
	{
		// there should exactly be one state change
		QCOMPARE( emits, 1 );
		QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
		Phonon::State newstate = qvariant_cast<Phonon::State>( args.at( 0 ) );
		Phonon::State oldstate = qvariant_cast<Phonon::State>( args.at( 1 ) );

		QCOMPARE( Phonon::LoadingState, oldstate );
		QCOMPARE( s, newstate );
		if( Phonon::ErrorState == s )
			QFAIL( "Loading the data put the ByteStream into the ErrorState. Check that PHONON_TESTURL is set to a valid URL." );
		QCOMPARE( Phonon::StoppedState, s );
		QCOMPARE( m_stateChangedSignalSpy->count(), 0 );

		// check for length signal
		QVERIFY( lengthSignalSpy.count() > 0 );
		args = lengthSignalSpy.takeLast();
		QCOMPARE( m_media->totalTime(), castQVariantToInt64( args.at( 0 ) ) );
	}
	else
	{
		QFAIL( "Still in LoadingState after a stateChange signal was emitted. Cannot go on." );
	}
}

void ByteStreamTest::checkForDefaults()
{
	QCOMPARE( m_media->tickInterval(), qint32( 0 ) );
	QCOMPARE( m_media->aboutToFinishTime(), qint32( 0 ) );
}

void ByteStreamTest::stopToStop()
{
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
	m_media->stop();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
}

void ByteStreamTest::stopToPause()
{
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
	m_media->pause();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::stopToPlay()
{
	startPlayback();
	stopPlayback( Phonon::PlayingState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::playToPlay()
{
	startPlayback();

	m_media->play();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::PlayingState );

	stopPlayback( Phonon::PlayingState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::playToPause()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );
	stopPlayback( Phonon::PausedState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::playToStop()
{
	startPlayback();
	stopPlayback( Phonon::PlayingState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::pauseToPause()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );

	m_media->pause();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::PausedState );

	stopPlayback( Phonon::PausedState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::pauseToPlay()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );
    startPlayback(Phonon::PausedState);
	stopPlayback( Phonon::PlayingState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::pauseToStop()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );
	stopPlayback( Phonon::PausedState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::testSeek()
{
	startPlayback();
	qint64 c = m_media->currentTime();
	qint64 r = m_media->remainingTime();
	if( m_media->isSeekable() )
		if( r > 0 )
		{
			qint64 s = c + r/2;
			QTime start = QTime::currentTime();
			m_media->seek( s );
			c = m_media->currentTime();
			r = m_media->remainingTime();
			QTime end = QTime::currentTime();
			QVERIFY( s <= c );
			QVERIFY( c <= s + start.msecsTo( end ) );

			s /= 2;
			start = QTime::currentTime();
			m_media->seek( s );
			c = m_media->currentTime();
			r = m_media->remainingTime();
			end = QTime::currentTime();
			QVERIFY( s <= c );
			QVERIFY( c <= s + start.msecsTo( end ) );

			pausePlayback( Phonon::PlayingState );
			s *= 2;
			m_media->seek( s );
			c = m_media->currentTime();
			QVERIFY( s == c );

			s /= 2;
			m_media->seek( s );
			c = m_media->currentTime();
			QVERIFY( s == c );
			stopPlayback( Phonon::PausedState );
			return;
		}
		else
			QWARN( "didn't test seeking as the ByteStream reported a remaining size <= 0" );
	else
		QWARN( "didn't test seeking as the ByteStream is not seekable" );
	stopPlayback( Phonon::PlayingState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::testAboutToFinish()
{
    const qint32 aboutToFinishTime = 2000;

    m_media->setAboutToFinishTime(aboutToFinishTime);
    QCOMPARE(m_media->aboutToFinishTime(), qint32(aboutToFinishTime));
	QSignalSpy aboutToFinishSpy( m_media, SIGNAL( aboutToFinish( qint32 ) ) );
	QSignalSpy finishSpy( m_media, SIGNAL( finished() ) );
	startPlayback();
    if(m_media->isSeekable()) {
        m_media->seek( m_media->totalTime() - 2000 - aboutToFinishTime ); // give it 2 seconds to
        // play until the signal needs to be emitted
    }
	while( aboutToFinishSpy.count() == 0 && ( m_media->state() == Phonon::PlayingState || m_media->state() == Phonon::BufferingState ) )
		QCoreApplication::processEvents();
	// at this point the media should be about to finish playing
	qint64 r = m_media->remainingTime();
	Phonon::State state = m_media->state();
	QCOMPARE( aboutToFinishSpy.count(), 1 );

    // allow it to be up to 150ms too early
    const qint32 received = castQVariantToInt32(aboutToFinishSpy.first().at(0));
    QVERIFY(received <= aboutToFinishTime + 150);
    if (state == Phonon::PlayingState || state == Phonon::BufferingState) {
        QVERIFY(r <= received);
		while( finishSpy.count() == 0 )
			QCoreApplication::processEvents();
    } else {
        QVERIFY(received > 0);
	}
	QCOMPARE( finishSpy.count(), 1 );

	QCOMPARE( m_stateChangedSignalSpy->count(), 1 );
	QList<QVariant> args = m_stateChangedSignalSpy->takeFirst();
	Phonon::State newstate = qvariant_cast<Phonon::State>( args.at( 0 ) );
	Phonon::State oldstate = qvariant_cast<Phonon::State>( args.at( 1 ) );
	QCOMPARE( oldstate, Phonon::PlayingState );
	QCOMPARE( newstate, Phonon::StoppedState );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
	initByteStream();
	initOutput();
	setMedia();
}

void ByteStreamTest::testTickSignal()
{
	for( qint32 tickInterval = 20; tickInterval <= 1000; tickInterval *= 2 )
	{
        QSignalSpy tickSpy(m_media, SIGNAL(tick(qint64)));
        QCOMPARE(m_media->tickInterval(), qint32(0));
		qDebug() << "Test 20 ticks with an interval of" <<  tickInterval << "ms";
		m_media->setTickInterval( tickInterval );
		QVERIFY( m_media->tickInterval() <= tickInterval );
		QVERIFY( m_media->tickInterval() >= tickInterval/2 );
		QTime start1 = QTime::currentTime();
		startPlayback();
		QTime start2 = QTime::currentTime();
		int lastCount = 0;
		qint64 s1, s2 = start2.elapsed();
		while( tickSpy.count() < 20 && ( m_media->state() == Phonon::PlayingState || m_media->state() == Phonon::BufferingState ) )
		{
            // FIXME if the ByteStream goes into BufferingState this will break. The timers have to
            // stop while it's buffering.
			if( tickSpy.count() > lastCount )
			{
				s1 = start1.elapsed();
				qint64 tickTime = castQVariantToInt64( tickSpy.last().at( 0 ) );
				lastCount = tickSpy.count();
				// s1 is the time from before the beginning of the playback to
				// after the tick signal
				// s2 is the time from after the beginning of the playback to
				// before the tick signal
				// so: s2 <= s1

				// allow +/-350ms inaccuracy
				const int inaccuracy = 350;

				QVERIFY( tickTime <= m_media->currentTime() );
				QVERIFY( s1 + inaccuracy >= tickTime );
				QVERIFY( s2 - inaccuracy <= tickTime );
				/*
				if( s1 + inaccuracy < tickTime )
					QWARN( qPrintable( QString( "tickTime %1 is greater than %2 - %3, the time that elapsed since before playback was started" )
								.arg( tickTime ).arg( s1 + inaccuracy ).arg( inaccuracy ) ) );
				if( s2 - inaccuracy > tickTime )
					QWARN( qPrintable( QString( "tickTime %1 is less than %2 + %3, the time that elapsed since after playback was started" )
								.arg( tickTime ).arg( s2 - inaccuracy ).arg( inaccuracy ) ) );
				*/
				QVERIFY( s1 >= lastCount * m_media->tickInterval() );
				if( s2 > ( lastCount + 1 ) * m_media->tickInterval() )
					QWARN( qPrintable( QString( "%1. tick came too late: %2ms elapsed while this tick should have come before %3ms" )
							.arg( lastCount ).arg( s2 ).arg( ( lastCount + 1 ) * m_media->tickInterval() ) ) );
            } else if (lastCount == 0 && s2 > 20 * m_media->tickInterval()) {
                QFAIL("no tick signals are being received");
            }
			s2 = start2.elapsed();
			QCoreApplication::processEvents();
		}
		stopPlayback( Phonon::PlayingState );
		initByteStream();
		initOutput();
		setMedia();
	}
}

void ByteStreamTest::addPaths()
{
	AudioPath *a1 = new AudioPath( this );
	AudioPath *a2 = new AudioPath( this );
	VideoPath *v1 = new VideoPath( this );
	VideoPath *v2 = new VideoPath( this );
	QCOMPARE( m_media->audioPaths().size(), 0 );
	QCOMPARE( m_media->videoPaths().size(), 0 );
	QVERIFY( m_media->addAudioPath( a1 ) );
	QCOMPARE( m_media->audioPaths().size(), 1 ); // one should always work
	QVERIFY( m_media->audioPaths().contains( a1 ) );
	QCOMPARE( m_media->addAudioPath( a1 ), false ); // adding the same one should not work
	QCOMPARE( m_media->audioPaths().size(), 1 );
	QVERIFY( m_media->audioPaths().contains( a1 ) );
	if( m_media->addAudioPath( a2 ) )
	{
		QCOMPARE( m_media->audioPaths().size(), 2 );
		QVERIFY( m_media->audioPaths().contains( a1 ) );
		QVERIFY( m_media->audioPaths().contains( a2 ) );
		QCOMPARE( m_media->addAudioPath( a1 ), false ); // adding the same one should not work
		QCOMPARE( m_media->audioPaths().size(), 2 );
		QCOMPARE( m_media->addAudioPath( a2 ), false ); // adding the same one should not work
		QCOMPARE( m_media->audioPaths().size(), 2 );
		delete a2;
		QCOMPARE( m_media->audioPaths().size(), 1 );
		QVERIFY( m_media->audioPaths().contains( a1 ) );
		QVERIFY( !m_media->audioPaths().contains( a2 ) );
		a2 = new AudioPath( this );
		QCOMPARE( m_media->addAudioPath( a2 ), true );
		QCOMPARE( m_media->audioPaths().size(), 2 );
		QVERIFY( m_media->audioPaths().contains( a1 ) );
		QVERIFY( m_media->audioPaths().contains( a2 ) );
		delete a2;
		QCOMPARE( m_media->audioPaths().size(), 1 );
		QVERIFY( m_media->audioPaths().contains( a1 ) );
		QVERIFY( !m_media->audioPaths().contains( a2 ) );
		a2 = 0;
	}
	else
		QWARN( "backend does not allow usage of more than one AudioPath" );
	delete a1;
	QCOMPARE( m_media->audioPaths().size(), 0 );
	a1 = 0;

	QVERIFY( m_media->addVideoPath( v1 ) );
	QCOMPARE( m_media->videoPaths().size(), 1 ); // one should always work
	QVERIFY( m_media->videoPaths().contains( v1 ) );
	QCOMPARE( m_media->addVideoPath( v1 ), false ); // adding the same one should not work
	QCOMPARE( m_media->videoPaths().size(), 1 );
	QVERIFY( m_media->videoPaths().contains( v1 ) );
	if( m_media->addVideoPath( v2 ) )
	{
		QCOMPARE( m_media->videoPaths().size(), 2 );
		QVERIFY( m_media->videoPaths().contains( v1 ) );
		QVERIFY( m_media->videoPaths().contains( v2 ) );
		QCOMPARE( m_media->addVideoPath( v1 ), false ); // adding the same one should not work
		QCOMPARE( m_media->videoPaths().size(), 2 );
		QCOMPARE( m_media->addVideoPath( v2 ), false ); // adding the same one should not work
		QCOMPARE( m_media->videoPaths().size(), 2 );
		delete v2;
		QCOMPARE( m_media->videoPaths().size(), 1 );
		QVERIFY( m_media->videoPaths().contains( v1 ) );
		QVERIFY( !m_media->videoPaths().contains( v2 ) );
		v2 = new VideoPath( this );
		QCOMPARE( m_media->addVideoPath( v2 ), true );
		QCOMPARE( m_media->videoPaths().size(), 2 );
		QVERIFY( m_media->videoPaths().contains( v1 ) );
		QVERIFY( m_media->videoPaths().contains( v2 ) );
		delete v2;
		QCOMPARE( m_media->videoPaths().size(), 1 );
		QVERIFY( m_media->videoPaths().contains( v1 ) );
		QVERIFY( !m_media->videoPaths().contains( v2 ) );
		v2 = 0;
	}
	else
		QWARN( "backend does not allow usage of more than one VideoPath" );
	delete v1;
	QCOMPARE( m_media->videoPaths().size(), 0 );
	v1 = 0;
}

void ByteStreamTest::initOutput()
{
	// AudioPath and AudioOutput are needed else the backend might finish in no time
	if( !m_audioPath && !m_audioOutput )
	{
		m_audioPath = new AudioPath( this );
		m_audioOutput = new AudioOutput( Phonon::MusicCategory, this );
		//m_audioOutput->setVolume( 0.0f );
		m_audioPath->addOutput( m_audioOutput );
	}
	QVERIFY( m_media->addAudioPath( m_audioPath ) );
}

void ByteStreamTest::cleanupTestCase()
{
	delete m_stateChangedSignalSpy;
	delete m_media;
}

QTEST_KDEMAIN( ByteStreamTest, GUI )
#include "bytestreamtest.moc"
// vim: sw=4 ts=4
