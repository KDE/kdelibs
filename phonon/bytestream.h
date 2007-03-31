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
#ifndef PHONON_BYTESTREAM_H
#define PHONON_BYTESTREAM_H

#include "abstractmediaproducer.h"
#include <kdelibs_export.h>
#include "phonondefs.h"

class QString;
class QStringList;

namespace Phonon
{
	class ByteStreamPrivate;

    /*
     * loud thinking about async parts of this interface
     *                     =============================
     *
     * Let us go over all the methods to make sure we will not get in trouble
     * (i.e. deadlocks or necessary processEvents calls)
     *
     * The following functions are safe because they don't have anything to do
     * with the stream data
     * - stop
     * - pause
     * - state
     * - currentTime
     * - videoPaths/audioPaths
     * - streamSize/setStreamSize
     * - tickInterval/setTickInterval
     * - streamSeekable/setStreamSeekable
     * - aboutToFinishTime/setAboutToFinishTime
     * - selectAudioStream/selectVideoStream/selectSubtitleStream
     * - selectedAudioStream/selectedVideoStream/selectedSubtitleStream
     *
     * - metaDataKeys/metaDataItem/metaDataItems
     *   Safe because the meta data is sent to the frontend async (calls don't
     *   call any functions on the backend object)
     *
     * - addVideoPath/addAudioPath
     *   flow graph setup calls, sometimes depend on stream data:
     *   With xine you need to create a new xine_stream which might need to do
     *   seeks and reads. In this case the backend should go into LoadingState
     *   or BufferingState (or ErrorState if the stream is not seekable).
     *
     * - hasVideo
     * - isSeekable
     * - availableAudioStreams/availableVideoStreams/availableSubtitleStreams
     *   Needs the stream data to know the "correct answer".
     *   Returns false if not enough data has been sent. TODO: might need a
     *   Q_SIGNAL to make it work async.
     *
     * - totalTime/remainingTime
     *   Needs the stream data to know the "correct answer".
     *   Returns -1 if not enough data has been sent. Has the length signal for
     *   async notification.
     *
     * - play
     * - seek
     *   if not enough data is available to start playback the backend goes into
     *   BufferingState
     */

	/**
	 * \short Send media data to be decoded and played back.
	 *
	 * This class allows you to send arbitrary media data (well, any media data
	 * format that the backend can decode) to the mediaframework used by the
	 * backend. MediaObject uses the facilities of this class internally when
	 * the mediaframework cannot handle an URL using KIO to fetch the data and
	 * ByteStream to stream it to the backend.
	 *
	 * To use it you need to provide a constant (fast enough) datastream via
	 * writeData. The signals needData and enoughData tell when the stream is
	 * about to drop out or when the internal buffer is sufficently filled. The
	 * backend should be able to buffer whatever you throw at it (of course,
	 * depending on the available memory of the computer).
	 *
	 * This allows your data provider to use a push method (calling writeData
	 * and waiting when enoughData is emitted) or a pull method (call writeData
	 * whenever needData is emitted).
	 *
	 * If your datastream is seekable you can tell the ByteStream object via
	 * setStreamSeekable. When a seek is called and the ByteStream needs data at
	 * a different position it will tell you with the seekStream signal. This
	 * signal passes the offset from the beginning of the datastream where the
	 * next array of data passed to writeData should start from.
	 *
	 * When the datastream is complete written you need to call endOfData, else
	 * ByteStream will not emit the finished signal, but go into the
	 * BufferingState.
	 *
	 * As soon as you know the size of the stream (in bytes) you should call
	 * setStreamSize. This could be useful for the ByteStream object to
	 * correctly emit the aboutToFinish signal. If the size is not known the
	 * aboutToFinish signal will still work, but you should not depend on it
	 * being emitted at the correct time.
	 *
     * \ingroup Playback
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT ByteStream : public AbstractMediaProducer
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( ByteStream )
		PHONON_HEIR( ByteStream )
		Q_PROPERTY( qint32 aboutToFinishTime READ aboutToFinishTime WRITE setAboutToFinishTime )
		Q_PROPERTY( qint64 streamSize READ streamSize WRITE setStreamSize )
		Q_PROPERTY( bool streamSeekable READ streamSeekable WRITE setStreamSeekable )
		public:
			/**
			 * \copydoc MediaObject::totalTime()
			 */
			qint64 totalTime() const;

			/**
			 * \copydoc MediaObject::remainingTime()
			 */
			qint64 remainingTime() const;

			/**
			 * \copydoc MediaObject::aboutToFinishTime()
			 */
			qint32 aboutToFinishTime() const;

			/**
			 * Returns the size of the stream in bytes. If the size is unknown
			 * \c -1 is returned.
			 *
			 * \return \c -1 if the size is unknown, otherwise the size in bytes
			 *
			 * \see setStreamSize
			 */
			qint64 streamSize() const;

			/**
			 * Returns whether you need to react on the seekStream signal when
			 * it is emitted.
			 *
			 * \return \c true You have to seek in the datastream when
			 * seekStream is emitted.
			 * \return \c false You only have to write the stream to writeData
			 * in one sequence.
			 *
			 * \see setStreamSeekable
			 */
			bool streamSeekable() const;

		public Q_SLOTS:
			/**
			 * Tell the object whether you will support seeking in the
			 * datastream. If you do, you have to react to the seekStream
			 * signal. If you don't you can safely ignore that signal.
			 *
			 * \param seekable Whether you are able to seek in the datastream
			 * and provide the writeData method with the data at arbitrary
			 * positions.
			 *
			 * \see streamSeekable
			 * \see seekStream
			 */
			void setStreamSeekable( bool seekable );

			/**
			 * Passes media data to the mediaframework used by the backend. The
			 * data should be written in chunks of at least 512 bytes to not
			 * become too inefficient.
			 *
			 * \todo Actually I have no idea what array size should be used.
			 * Some tests would be good.
			 *
			 * \param data A QByteArray holding the data. After this call you are free
			 * to dispose of the array.
			 *
			 * \see needData
			 * \see enoughData
			 * \see seekStream
			 */
			void writeData( const QByteArray& data );

			/**
			 * Sets how many bytes the complete datastream has. The Size is
			 * counted from the start of the stream until the end and not from
			 * the point in time when the slot is called.
			 *
			 * \param streamSize The size of the stream in bytes. The special
			 * value \c -1 is used for "unknown size".
			 * 
			 * \see streamSize
			 * \see endOfData
			 */
			void setStreamSize( qint64 streamSize );

			/**
			 * Call this slot after your last call to writeData. This is needed
			 * for the ByteStream to properly emit the finished and
			 * aboutToFinish signals. If the internal buffer is too small to
			 * cover the aboutToFinishTime the streamSize is used for the
			 * aboutToFinish signal. If the streamSize is unknown and the buffer
			 * is too small to emit the aboutToFinish signal in time it is
			 * emitted as soon as possible.
			 *
			 * \see setStreamSize
			 */
			void endOfData();

			/**
			 * \copydoc MediaObject::setAboutToFinishTime( qint32 )
			 */
			void setAboutToFinishTime( qint32 newAboutToFinishTime );

		Q_SIGNALS:
			/**
			 * \copydoc MediaObject::finished()
			 */
			void finished();

			/**
			 * Emitted when the stream is about to finish. The aboutToFinishTime
			 * is not guaranteed to be correct as sometimes the stream time
			 * until the end is not known early enough. Never rely on the
			 * aboutToFinishTime you set, but use the \p msec parameter instead.
			 *
			 * \warning Sometimes the media framework has no idea when the
			 * stream will end and only knows it when it has finished. For
			 * example, for an Ogg Vorbis stream which is not seekable the
			 * backend can only estimate the length of the stream by looking at
			 * the average bitrate and size of the stream, which can make a big
			 * difference some times. So especially when the stream is not
			 * seekable you cannot expect this signal to be of any use.
			 *
			 * \param msec The time in milliseconds until the stream finishes.
			 */
			void aboutToFinish( qint32 msec );

			/**
			 * \copydoc MediaObject::length()
			 */
			void length( qint64 length );

			/**
			 * Emitted when the ByteStream object needs more data to process.
			 * Your slot should not take too long to call writeData because otherwise
			 * the stream might drop.
			 */
			void needData();

			/**
			 * Emitted when the ByteStream object has enough data to process.
			 * This means you do not need to call writeData anymore until
			 * needData is emitted.
			 */
			void enoughData();

			/**
			 * Emitted when the ByteStream needs you to continue streaming data
			 * at a different position in the stream. This happens when seek(
			 * qint64 ) is called and the needed data is not in the internal
			 * buffer.
			 *
			 * \param age The number of bytes since the start of the stream.
			 *            Your next call to writeData is expected to contain the
			 *            data from this position on.
			 *
			 * \see setStreamSeekable
			 */
			void seekStream( qint64 age );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_BYTESTREAM_H
