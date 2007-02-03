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

#include "bytestreamtest2.h"
#include "qtesthelper.h"
#include <cstdlib>
#include <qtest_kde.h>
#include <QTime>
#include <QtDebug>
#include <phonon/videopath.h>

using namespace Phonon;

Q_DECLARE_METATYPE( Phonon::State )

static const qint64 castQVariantToInt64( const QVariant& variant )
{
	return *reinterpret_cast<const qint64*>( variant.constData() );
}

static const qint32 castQVariantToInt32( const QVariant& variant )
{
	return *reinterpret_cast<const qint32*>( variant.constData() );
}

void ByteStreamTest2::startPlayback(Phonon::State currentState)
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

void ByteStreamTest2::stopPlayback( Phonon::State currentState )
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

void ByteStreamTest2::pausePlayback( Phonon::State currentState )
{
    QCOMPARE(m_stateChangedSignalSpy->count(), 0);
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

static const qint64 STREAM_SIZE = 1024 * 1024 * 4; // 4MB

void ByteStreamTest2::initTestCase()
{
	qRegisterMetaType<Phonon::State>( "Phonon::State" );
	qRegisterMetaType<qint32>( "qint32" );
	qRegisterMetaType<qint64>( "qint64" );

    // init timer that pushes the PCM data
	m_timer = new QTimer( this );
	m_timer->setInterval( 0 );
	connect( m_timer, SIGNAL( timeout() ), SLOT( sendBlock() ) );

	m_media = new ByteStream( this );
	m_media->setStreamSeekable( true );
	connect( m_media, SIGNAL( seekStream( qint64 ) ), SLOT( seekStream( qint64 ) ) );
	connect( m_media, SIGNAL( needData() ), m_timer, SLOT( start() ) );
	connect( m_media, SIGNAL( enoughData() ), m_timer, SLOT( stop() ) );

	m_stateChangedSignalSpy = new QSignalSpy( m_media, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ) );
	QVERIFY( m_stateChangedSignalSpy->isValid() );
	m_stateChangedSignalSpy->clear();
}

void ByteStreamTest2::sendBlock()
{
	if( m_position == 0 )
	{
		QByteArray block = wavHeader();
		m_position += block.size();
		m_media->writeData( block );
	}

	QByteArray block = pcmBlock();
	m_position += block.size();
	if( m_position > STREAM_SIZE )
	{
		m_position -= block.size();
		block = block.left( STREAM_SIZE - m_position );
		m_position += block.size();
		Q_ASSERT( m_position == STREAM_SIZE );
	}
	m_media->writeData( block );
	if( m_position == STREAM_SIZE )
	{
		m_media->endOfData();
		m_timer->stop();
	}
}

void ByteStreamTest2::seekStream( qint64 newPos )
{
	Q_ASSERT( newPos <= STREAM_SIZE );
	m_position = newPos;
}

void ByteStreamTest2::setMedia()
{
	QSignalSpy lengthSignalSpy( m_media, SIGNAL( length( qint64 ) ) );
	QCOMPARE( m_media->state(), Phonon::LoadingState );
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );

    // send the WAV header and then push the PCM data into the stream until the stream says it got
    // enough
	m_media->setStreamSize( STREAM_SIZE );
	QByteArray block = wavHeader();
	m_position += block.size();
	m_media->writeData( block );
	m_timer->start();

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
            QFAIL("Loading the data put the ByteStream into the ErrorState. Apparently the backend does not support WAV data.");
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

void ByteStreamTest2::checkForDefaults()
{
	QCOMPARE( m_media->tickInterval(), qint32( 0 ) );
	QCOMPARE( m_media->aboutToFinishTime(), qint32( 0 ) );
}

void ByteStreamTest2::stopToStop()
{
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
	m_media->stop();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
}

void ByteStreamTest2::stopToPause()
{
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
	m_media->pause();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::StoppedState );
}

void ByteStreamTest2::stopToPlay()
{
	startPlayback();
	stopPlayback( Phonon::PlayingState );
}

void ByteStreamTest2::playToPlay()
{
	startPlayback();

	m_media->play();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::PlayingState );

	stopPlayback( Phonon::PlayingState );
}

void ByteStreamTest2::playToPause()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );
	stopPlayback( Phonon::PausedState );
}

void ByteStreamTest2::playToStop()
{
	startPlayback();
	stopPlayback( Phonon::PlayingState );
}

void ByteStreamTest2::pauseToPause()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );

	m_media->pause();
	QCOMPARE( m_stateChangedSignalSpy->count(), 0 );
	QCOMPARE( m_media->state(), Phonon::PausedState );

	stopPlayback( Phonon::PausedState );
}

void ByteStreamTest2::pauseToPlay()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );
    startPlayback(Phonon::PausedState);
	stopPlayback( Phonon::PlayingState );
}

void ByteStreamTest2::pauseToStop()
{
	startPlayback();
	pausePlayback( Phonon::PlayingState );
	stopPlayback( Phonon::PausedState );
}

void ByteStreamTest2::testSeek()
{
	startPlayback();
	qint64 c = m_media->currentTime();
	qint64 r = m_media->remainingTime();
    // the wav really should be seekable
    QVERIFY(m_media->isSeekable());
    if (r > 0) {
        qint64 s = c + r/2;
        QTime start = QTime::currentTime();
        m_media->seek(s);
        c = m_media->currentTime();
        r = m_media->remainingTime();
        QTime end = QTime::currentTime();
        QVERIFY(s <= c);
        QVERIFY(c <= s + start.msecsTo(end));

        s /= 2;
        start = QTime::currentTime();
        m_media->seek(s);
        c = m_media->currentTime();
        r = m_media->remainingTime();
        end = QTime::currentTime();
        QVERIFY(s <= c);
        QVERIFY(c <= s + start.msecsTo(end));

        pausePlayback(Phonon::PlayingState);
        s *= 2;
        m_media->seek(s);
        c = m_media->currentTime();
        QVERIFY(s == c);

        s /= 2;
        m_media->seek(s);
        c = m_media->currentTime();
        QVERIFY(s == c);
        stopPlayback(Phonon::PausedState);
        return;
    } else {
        QWARN("didn't test seeking as the ByteStream reported a remaining size <= 0");
    }
    stopPlayback(Phonon::PlayingState);
}

void ByteStreamTest2::testAboutToFinish()
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
}

void ByteStreamTest2::testTickSignal()
{
	QSignalSpy tickSpy( m_media, SIGNAL( tick( qint64 ) ) );
	QCOMPARE( m_media->tickInterval(), qint32( 0 ) );
	for( qint32 tickInterval = 20; tickInterval <= 1000; tickInterval *= 2 )
	{
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
		tickSpy.clear();
	}
}

void ByteStreamTest2::addPaths()
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

void ByteStreamTest2::initOutput()
{
	// AudioPath and AudioOutput are needed else the backend might finish in no time
	if( !m_audioPath && !m_audioOutput )
	{
		m_audioPath = new AudioPath( this );
		m_audioOutput = new AudioOutput( Phonon::MusicCategory, this );
		m_audioPath->addOutput( m_audioOutput );
	}
	QVERIFY( m_media->addAudioPath( m_audioPath ) );
}

void ByteStreamTest2::cleanupTestCase()
{
	delete m_stateChangedSignalSpy;
	delete m_media;
}

QByteArray ByteStreamTest2::wavHeader() const
{
	QByteArray data;
	QDataStream stream( &data, QIODevice::WriteOnly );
	stream.setByteOrder( QDataStream::LittleEndian );
	stream << 0x46464952 //"RIFF"
		<< static_cast<quint32>( STREAM_SIZE )
		<< 0x45564157 //"WAVE"
		<< 0x20746D66 //"fmt "           //Subchunk1ID
		<< static_cast<quint32>( 16 )    //Subchunk1Size
		<< static_cast<quint16>( 1 )     //AudioFormat
		<< static_cast<quint16>( 2 )     //NumChannels
		<< static_cast<quint32>( 48000 ) //SampleRate
		<< static_cast<quint32>( 2*2*48000 )//ByteRate
		<< static_cast<quint16>( 2*2 )   //BlockAlign
		<< static_cast<quint16>( 16 )    //BitsPerSample
		<< 0x61746164 //"data"                   //Subchunk2ID
		<< static_cast<quint32>( STREAM_SIZE-36 )//Subchunk2Size
		;
	return data;
}

QByteArray ByteStreamTest2::pcmBlock() const
{
	QByteArray data;
	QDataStream stream( &data, QIODevice::WriteOnly );
	stream.setByteOrder( QDataStream::LittleEndian );

	for( int x = 0; x < 64; ++x )
	{
		for( int i = 0; i < 32; ++i )
			stream << static_cast<quint16>( i * 1024 )
				<< static_cast<quint16>( -i * 1024 );
		for( int i = 0; i < 64; ++i )
			stream << static_cast<quint16>( 32767 - i * 1024 )
				<< static_cast<quint16>( -32768 + i * 1024 );
		for( int i = 0; i < 32; ++i )
			stream << static_cast<quint16>( -32768 + i * 1024 )
				<< static_cast<quint16>( 32767 - i * 1024 );
	}

	return data;
}

QTEST_KDEMAIN( ByteStreamTest2, GUI )
#include "bytestreamtest2.moc"
// vim: sw=4 ts=4
