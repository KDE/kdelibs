/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef Phonon_SIMPLEPLAYER_H
#define Phonon_SIMPLEPLAYER_H

#include <QObject>
#include "phononnamespace.h"
#include <kdelibs_export.h>

class KUrl;

namespace Phonon
{

/**
 * \short Playback class for simple tasks.
 *
 * With %SimplePlayer you can get results quickly and easily. You can do the standard
 * playback tasks like play, pause and stop, but also set a playback volume and
 * seek (there's no guarantee that the seek will work, though).
 *
 * Keep in mind that when the %SimplePlayer instance is deleted the playback will
 * stop.
 *
 * A play and forget code example:
 * \code
 * SimplePlayer* player = new SimplePlayer( Phonon::NotificationCategory );
 * connect( player, SIGNAL( finished() ), player, SLOT( deleteLater() ) );
 * player->play( url );
 * \endcode
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONCORE_EXPORT SimplePlayer : public QObject
{
	Q_OBJECT
	public:
		/**
		 * Constructs a new %SimplePlayer instance.
		 *
		 * \param category The category used for the audio output device.
		 * \param parent The QObject parent.
		 */
		SimplePlayer( Phonon::Category category, QObject* parent = 0 );
		~SimplePlayer();

		/**
		 * Get the total time (in milliseconds) of the file currently being played.
		 */
		qint64 totalTime() const;
		/**
		 * Get the current time (in milliseconds) of the file currently being played.
		 */
		qint64 currentTime() const;
		/**
		 * This is the current volume of the output as voltage factor.
		 *
		 * 1.0 means 100%, 0.5 means 50% voltage/25% power, 0.0 means 0%
		 */
		float volume() const;

		/**
		 * \returns \c true if it is currently playing
		 * \returns \c false if it is currently stopped or paused
		 */
		bool isPlaying() const;
		/**
		 * \returns \c true if it is currently paused
		 * \returns \c false if it is currently playing or stopped
		 */
		bool isPaused() const;

	public Q_SLOTS:
		/**
		 * Play the media at the given URL. Starts playback as fast as possible.
		 * This can take a considerable time depending on the URL and the
		 * backend.
		 *
		 * If you need low latency between calling play() and the sound actually
		 * starting to play on your output device you need to use MediaObject
		 * and be able to set the URL before calling play(). Note that
		 * \code
		 * mediaObject->setUrl( url );
		 * mediaObject->play();
		 * \endcode
		 * doesn't make a difference, the application should be idle between the
		 * setUrl and play calls so that the backend can start preloading the
		 * media and fill audio buffers.
		 */
		void play( const KUrl& url );
		/**
		 * Continues playback of a paused media. Restarts playback of a stopped
		 * media.
		 */
		void play();
		/**
		 * Pauses the playback.
		 */
		void pause();
		/**
		 * Stops the playback.
		 */
		void stop();

		/**
		 * Seeks to the requested time. Note that the backend is free to ignore
		 * the seek request if the media source isn't seekable.
		 *
		 * \param ms Time in milliseconds from the start of the media.
		 */
		void seek( qint64 ms );
		/**
		 * Sets the volume of the output as voltage factor.
		 *
		 * 1.0 means 100%, 0.5 means 50% voltage/25% power, 0.0 means 0%
		 */
		void setVolume( float volume );

	Q_SIGNALS:
		/**
		 * This signal is emitted when the playback finished.
		 */
		void finished();

	private Q_SLOTS:
		void stateChanged( Phonon::State, Phonon::State );

	private:
		class Private;
		Private * d;
};

} //namespace Phonon

#endif // Phonon_SIMPLEPLAYER_H
// vim: sw=4 ts=4 noet tw=80
