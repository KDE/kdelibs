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
#ifndef Phonon_IFACES_ABSTRACTMEDIAPRODUCER_H
#define Phonon_IFACES_ABSTRACTMEDIAPRODUCER_H

#include "phonon/phononnamespace.h"
#include "base.h"

class QObject;
template<class T> class QList;

namespace Phonon
{
namespace Ifaces
{
	class VideoPath;
	class AudioPath;

	/**
	 * \brief Base class for media sources.
	 *
	 * This class holds common methods for all "media producing" classes.
	 *
	 * \see Phonon::AbstractMediaProducer
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class AbstractMediaProducer : public Base
	{
		public:
			/**
			 * Adds a VideoPath object to tell where to send the video data (and
			 * render the subtitle if one is selected).
			 *
			 * The frontend class ensures that the \p videoPath is unique,
			 * meaning the VideoPath object has not been added to this object.
			 */
			virtual bool addVideoPath( VideoPath* videoPath ) = 0;

			/**
			 * Adds an AudioPath object to tell where to send the audio data.
			 *
			 * The frontend class ensures that the \p audioPath is unique,
			 * meaning the AudioPath object has not been added to this object.
			 */
			virtual bool addAudioPath( AudioPath* audioPath ) = 0;

			virtual void removeVideoPath( VideoPath* videoPath ) = 0;
			virtual void removeAudioPath( AudioPath* audioPath ) = 0;

			/**
			 * Get the current state.
			 */
			virtual State state() const = 0;

			/**
			 * Check whether the media data includes a video stream.
			 *
			 * @return returns \p true if the media contains video data
			 */
			virtual bool hasVideo() const = 0;

			/**
			 * If the current media may be seeked returns true.
			 *
			 * @returns whether the current media may be seeked.
			 */
			virtual bool seekable() const = 0;

			/**
			 * Get the current time (in milliseconds) of the file currently being played.
			 */
			virtual long currentTime() const = 0;

			/**
			 * Return the time interval in milliseconds between two ticks.
			 *
			 * @returns Returns the tick intervall that it was set to (might not
			 * be the same as you asked for).
			 */
			virtual long tickInterval() const = 0;

			virtual QStringList availableAudioStreams() const = 0;
			virtual QStringList availableVideoStreams() const = 0;
			virtual QStringList availableSubtitleStreams() const = 0;

			virtual QString selectedAudioStream( const AudioPath* audioPath ) const = 0;
			virtual QString selectedVideoStream( const VideoPath* videoPath ) const = 0;
			virtual QString selectedSubtitleStream( const VideoPath* videoPath ) const = 0;

			virtual void selectAudioStream( const QString& streamName, const AudioPath* audioPath ) = 0;
			virtual void selectVideoStream( const QString& streamName, const VideoPath* videoPath ) = 0;
			virtual void selectSubtitleStream( const QString& streamName, const VideoPath* videoPath ) = 0;

			/**
			 * Change the interval the tick signal is emitted. If you set \p
			 * msec to 0 the signal gets disabled.
			 *
			 * \param newTickInterval tick interval in milliseconds
			 *
			 * @returns Returns the tick intervall that it was set to (might not
			 * be the same as you asked for).
			 */
			virtual long setTickInterval( long newTickInterval ) = 0;

			/**
			 * Play the media data.
			 */
			virtual void play() = 0;

			/**
			 * Pause a playing media. If it was paused before nothing changes.
			 */
			virtual void pause() = 0;

			/**
			 * Stop a playback.
			 */
			virtual void stop() = 0;

			/**
			 * Seek to the time indicated.
			 *
			 * @param time The time in milliseconds where to continue playing.
			 */
			virtual void seek( long time ) = 0;

		//Q_SIGNALS:
			/**
			 * Emitted when the state of the MediaObject has changed.
			 * In case you're not interested in the old state you can also
			 * connect to a slot that only has one State argument.
			 *
			 * @param newstate The state the Player is in now.
			 * @param oldstate The state the Player was in before.
			 */
			virtual void stateChanged( Phonon::State newstate, Phonon::State oldstate ) = 0;

			/**
			 * This signal gets emitted every tickInterval milliseconds.
			 *
			 * \param time The position of the media file in milliseconds.
			 *
			 * \see setTickInterval, tickInterval
			 */
			virtual void tick( long time ) = 0;
	};
}} //namespace Phonon::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // Phonon_IFACES_ABSTRACTMEDIAPRODUCER_H
