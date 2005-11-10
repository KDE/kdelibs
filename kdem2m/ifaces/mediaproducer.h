/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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
#ifndef KDEM2M_IFACES_MEDIAPRODUCER_H
#define KDEM2M_IFACES_MEDIAPRODUCER_H

#include "kdem2m/state.h"

class QObject;
template<class T> class QList;

namespace KDEM2M
{
namespace Ifaces
{
	class VideoPath;
	class AudioPath;

	class MediaProducer
	{
		public:
			MediaProducer() {}
			virtual ~MediaProducer() {}

			// Operations:
			virtual bool addVideoPath( VideoPath* videoPath ) = 0;
			virtual bool addAudioPath( AudioPath* audioPath ) = 0;

			// Attributes Getters:
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
			 
			 * Change the interval the tick signal is emitted. If you set \p
			 * msec to 0 the signal gets disabled.
			 *
			 * \param msec tick interval in milliseconds
			 *
			 * @returns Returns the tick intervall that it was set to (might not
			 * be the same as you asked for).
			 */
			virtual long tickInterval() const = 0;

			// Attributes Setters:
			/**
			 * Return the time interval in milliseconds between two ticks.
			 
			 * Change the interval the tick signal is emitted. If you set \p
			 * msec to 0 the signal gets disabled.
			 *
			 * \param msec tick interval in milliseconds
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

		//signals:
			/**
			 * Emitted when the state of the MediaObject has changed.
			 * In case you're not interested in the old state you can also
			 * connect to a slot that only has one State argument.
			 *
			 * @param self A pointer to the MediaObject that emitted the signal.
			 *             This can be usefull if you have several MediaObjects.
			 * @param newstate The state the Player is in now.
			 * @param oldstate The state the Player was in before.
			 */
			virtual void stateChanged( KDEM2M::State newstate, KDEM2M::State oldstate ) = 0;
			/**
			 * This signal gets emitted every tickInterval milliseconds.
			 *
			 * \param time The position of the media file in milliseconds.
			 *
			 * \see setTickInterval, tickInterval
			 */
			virtual void tick( long time ) = 0;

		public:
			virtual QObject* qobject() = 0;
			virtual const QObject* qobject() const = 0;

		private:
			class Private;
			Private* d;
	};
}} //namespace KDEM2M::Ifaces

// vim: sw=4 ts=4 tw=80 noet
#endif // KDEM2M_IFACES_MEDIAPRODUCER_H
