/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#ifndef PHONON_ABSTRACTMEDIAPRODUCER_H
#define PHONON_ABSTRACTMEDIAPRODUCER_H

#include "base.h"
#include "phononnamespace.h"
#include "phonondefs.h"
#include <QObject>
#include "basedestructionhandler.h"

template<class T> class QList;

namespace Phonon
{
	class VideoPath;
	class AudioPath;
	class AbstractMediaProducerPrivate;

	namespace Ifaces
	{
		class AbstractMediaProducer;
	}

	/**
	 * @short Common interface for all media sources.
	 *
	 * This class holds common functionality of all objects that produce a media
	 * stream, like MediaObject or SoundcardCapture.
	 *
	 * @author Matthias Kretz <kretz@kde.org>
	 * @see MediaObject
	 */
	class PHONONCORE_EXPORT AbstractMediaProducer : public QObject, public Base, private BaseDestructionHandler
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( AbstractMediaProducer )
		PHONON_ABSTRACTBASE( AbstractMediaProducer )
		public:
			~AbstractMediaProducer();

			/**
			 * Add a VideoPath to process the video data of this media.
			 *
			 * The interface allows for a VideoPath to be added to multiple
			 * AbstractMediaProducer objects and multiple VideoPath objects to
			 * be added to an AbstractMediaProducer object.
			 *
			 * @warning Backends don't have to be able to add multiple
			 * VideoPaths or use a VideoPath for multiple AbstractMediaProducer
			 * objects. If in doubt take care of the return value.
			 *
			 * @param videoPath The VideoPath that should process the video
			 * data.
			 *
			 * @return @c true if the VideoPath was successfully added
			 * @return @c false if the VideoPath could not be added. This can
			 * happen if the backend does not support multiple paths per
			 * AbstractMediaProducer or VideoPath for multiple
			 * AbstractMediaProducers.
			 *
			 * @see videoPaths
			 */
			bool addVideoPath( VideoPath* videoPath );

			/**
			 * Add an AudioPath to process the audio data of this media.
			 *
			 * The interface allows for an AudioPath to be added to multiple
			 * AbstractMediaProducer objects and multiple AudioPath objects to
			 * be added to an AbstractMediaProducer object.
			 *
			 * @warning Backends don't have to be able to add multiple
			 * AudioPaths or use a AudioPath for multiple AbstractMediaProducer
			 * objects. If in doubt take care of the return value.
			 *
			 * @param audioPath The AudioPath that should process the audio
			 * data.
			 *
			 * @return @c true if the AudioPath was successfully added
			 * @return @c false if the AudioPath could not be added. This can
			 * happen if the backend does not support multiple paths per
			 * AbstractMediaProducer or AudioPath for multiple
			 * AbstractMediaProducers.
			 *
			 * @see audioPaths
			 */
			bool addAudioPath( AudioPath* audioPath );

			/**
			 * Get the current state.
			 *
			 * @return The state of the object.
			 *
			 * @see State
			 */
			State state() const;

			/**
			 * Check whether the media data includes a video stream.
			 *
			 * @return Returns @c true if the media contains video data.
			 */
			bool hasVideo() const;

			/**
			 * If the current media may be seeked returns true.
			 *
			 * @return Whether the current media may be seeked.
			 */
			bool seekable() const;

			/**
			 * Get the current time (in milliseconds) of the file currently being played.
			 *
			 * @return The current time (in milliseconds)
			 */
			long currentTime() const;

			// TODO: document the freedom of the backend to choose the
			// tickInterval
			/**
			 * Return the time interval in milliseconds between two ticks.
			 *
			 * @return Returns the tick intervall that it was set to (might not
			 * be the same as you asked for, but then it will be less but more
			 * than half of what you asked for).
			 *
			 * @see setTickInterval
			 * @see tick
			 */
			long tickInterval() const;

			/**
			 * Returns the list of all connected VideoPath objects.
			 *
			 * @return A list of all connected VideoPath objects.
			 *
			 * @see addVideoPath
			 */
			const QList<VideoPath*>& videoPaths() const;

			/**
			 * Returns the list of all connected AudioPath objects.
			 *
			 * @return A list of all connected AudioPath objects.
			 *
			 * @see addAudioPath
			 */
			const QList<AudioPath*>& audioPaths() const;

		public Q_SLOTS:
			/**
			 * Change the interval the tick signal is emitted. If you set
			 * @p newTickInterval to @c 0 the signal gets disabled. The tick
			 * interval defaults to @c 0, meaning it is disabled.
			 *
			 * @param newTickInterval tick interval in milliseconds
			 *
			 * @see tickInterval
			 * @see tick
			 */
			void setTickInterval( long newTickInterval );

		public Q_SLOTS:
			/**
			 * Start playback of the media data.
			 */
			void play();

			/**
			 * Pause a playing media. If it was paused before nothing changes.
			 */
			void pause();

			/**
			 * Stop a playback. If it was stopped before nothing changes.
			 */
			void stop();

			/**
			 * Seek to the time indicated.
			 *
			 * You can only seek if playing, buffering or in pause.
			 *
			 * @param time The time in milliseconds where to continue playing.
			 */
			void seek( long time );

		Q_SIGNALS:
			/**
			 * Emitted when the state of the MediaObject has changed.
			 * In case you're not interested in the old state you can also
			 * connect to a slot that only has one State argument.
			 *
			 * @param newstate The state the Player is in now.
			 * @param oldstate The state the Player was in before.
			 */
			void stateChanged( Phonon::State newstate, Phonon::State oldstate );

			/**
			 * This signal gets emitted every tickInterval milliseconds.
			 *
			 * @param time The position of the media file in milliseconds.
			 *
			 * @see setTickInterval, tickInterval
			 */
			void tick( long time );

		private Q_SLOTS:
			void resumePlay();
			void resumePause();

		private:
			/**
			 * \internal
			 */
			virtual void phononObjectDestroyed( Base* );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_ABSTRACTMEDIAPRODUCER_H
