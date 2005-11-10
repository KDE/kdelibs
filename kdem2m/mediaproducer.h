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
#ifndef Kdem2m_MEDIAPRODUCER_H
#define Kdem2m_MEDIAPRODUCER_H

#include <QObject>

#include "state.h"

template<class T> class QList;

namespace Kdem2m
{
	class VideoPath;
	class AudioPath;

	namespace Ifaces
	{
		class MediaProducer;
	}

	class MediaProducer : public QObject
	{
		Q_OBJECT
		public:
			MediaProducer( QObject* parent = 0 );
			~MediaProducer();

			// Operations:
			bool addVideoPath( VideoPath* videoPath );
			bool addAudioPath( AudioPath* audioPath );

			// Attributes Getters:
			/**
			 * Get the current state.
			 */
			State state() const;
			/**
			 * Check whether the media data includes a video stream.
			 *
			 * @return returns \p true if the media contains video data
			 */
			bool hasVideo() const;
			/**
			 * If the current media may be seeked returns true.
			 *
			 * @returns whether the current media may be seeked.
			 */
			bool seekable() const;
			/**
			 * Get the current time (in milliseconds) of the file currently being played.
			 */
			long currentTime() const;
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
			long tickInterval() const;
			const QList<VideoPath*>& videoPaths() const;
			const QList<AudioPath*>& audioPaths() const;

		public slots:
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
			void setTickInterval( long newTickInterval );

		public slots:
			/**
			 * Play the media data.
			 */
			void play();
			/**
			 * Pause a playing media. If it was paused before nothing changes.
			 */
			void pause();
			/**
			 * Stop a playback.
			 */
			void stop();
			/**
			 * Seek to the time indicated.
			 *
			 * @param time The time in milliseconds where to continue playing.
			 */
			void seek( long time );

		signals:
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
			void stateChanged( Kdem2m::State newstate, Kdem2m::State oldstate );
			/**
			 * This signal gets emitted every tickInterval milliseconds.
			 *
			 * \param time The position of the media file in milliseconds.
			 *
			 * \see setTickInterval, tickInterval
			 */
			void tick( long time );

		protected:
			MediaProducer( Ifaces::MediaProducer* iface, QObject* parent );
			virtual bool aboutToDeleteIface();
			virtual void ifaceDeleted();
			virtual Ifaces::MediaProducer* createIface( bool initialized = true ) = 0;
			virtual void setupIface();

		protected slots:
			// when the Factory wants to change the Backend the following slots are used
			void slotDeleteIface();
			void slotCreateIface();

		private:
			Ifaces::MediaProducer* iface();
			Ifaces::MediaProducer* m_iface;
			class Private;
			Private* d;
	};
} //namespace Kdem2m

// vim: sw=4 ts=4 tw=80 noet
#endif // Kdem2m_MEDIAPRODUCER_H
