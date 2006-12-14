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

#ifndef MEDIAOBJECTTEST_H
#define MEDIAOBJECTTEST_H

#include <QObject>
#include <kurl.h>
#include <phonon/mediaobject.h>
#include <QtTest/QSignalSpy>

class MediaObjectTest : public QObject
{
	Q_OBJECT

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

		KUrl m_url;
		Phonon::MediaObject* m_media;
		QSignalSpy* m_stateChangedSignalSpy;
};

// vim: sw=4 ts=4
#endif // MEDIAOBJECTTEST_H
