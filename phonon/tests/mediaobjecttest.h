/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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

#ifndef MEDIAOBJECTTEST_H
#define MEDIAOBJECTTEST_H

#include <QObject>
#include <kurl.h>
#include <phonon/mediaobject.h>
#include <QtTest/QSignalSpy>

class MediaObjectTest : public QObject
{
	Q_OBJECT

    Q_SIGNALS:
        void continueTestPlayOnFinish();
    protected Q_SLOTS:
        void setMediaAndPlay();

	private Q_SLOTS:
        void init();
        void cleanup();

		void initTestCase();
		void checkForDefaults();

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

        void testTickSignal();
		void testSeek();
		void testAboutToFinish();
        void testPlayOnFinish();
        void testPlayBeforeFinish();

		void cleanupTestCase();

	private:
        void setMedia();
        void addPaths();
        void initOutput();

        void startPlayback(Phonon::State currentState = Phonon::StoppedState);
		void stopPlayback( Phonon::State currentState );
        void pausePlayback();
        void waitForSignal(QObject *obj, const char *signalName, int timeout = 0);
        void testOneSeek(qint64 seekTo);

		KUrl m_url;
		Phonon::MediaObject* m_media;
		QSignalSpy* m_stateChangedSignalSpy;
};

// vim: sw=4 ts=4
#endif // MEDIAOBJECTTEST_H
