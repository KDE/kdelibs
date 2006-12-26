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

#ifndef BYTESTREAMTEST2_H
#define BYTESTREAMTEST2_H

#include <QObject>
#include <kurl.h>
#include <phonon/bytestream.h>
#include <phonon/audiopath.h>
#include <phonon/audiooutput.h>
#include <QtTest/QSignalSpy>
#include <QTimer>

class DataProducer;

class ByteStreamTest2 : public QObject
{
    friend class DataProducer;

	Q_OBJECT
	public:
		ByteStreamTest2()
			: m_media( 0 ),
			m_stateChangedSignalSpy( 0 ),
			m_audioPath( 0 ),
			m_audioOutput( 0 ),
			m_position( 0 )
		{}

	private Q_SLOTS:
		void initTestCase();
		void setMedia();
		void checkForDefaults();

		void addPaths();
		void initOutput();

		// state change tests
		void stopToStop();
		void stopToPause();
		void stopToPlay();
		void playToPlay();
		void playToPause();
		void playToStop();
		void pauseToPause();
		void pauseToPlay();
		void pauseToStop();

		void testSeek();
		void testAboutToFinish();
		void testTickSignal();

		void cleanupTestCase();

	private:
        void startPlayback(Phonon::State currentState = Phonon::StoppedState);
		void stopPlayback( Phonon::State currentState );
		void pausePlayback( Phonon::State currentState );

		Phonon::ByteStream* m_media;
		QSignalSpy* m_stateChangedSignalSpy;
		Phonon::AudioPath* m_audioPath;
		Phonon::AudioOutput* m_audioOutput;
		qint64 m_position;
        DataProducer *m_dp;
};

class DataProducer : public QObject
{
    Q_OBJECT
    public:
        DataProducer(ByteStreamTest2 *parent);
        QByteArray wavHeader() const;

    public slots:
        void seekStream(qint64);
        void start();
        void stop();

    private slots:
        void sendBlock();

    private:
        QByteArray pcmBlock() const;

        QTimer *m_timer;
        ByteStreamTest2 *m_test;
};

// vim: sw=4 ts=4
#endif // BYTESTREAMTEST2_H
