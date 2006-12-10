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

#ifndef PHONON_VIDEODATAOUTPUT_H
#define PHONON_VIDEODATAOUTPUT_H

#include <kdelibs_export.h>
#include "abstractvideooutput.h"
#include "phonondefs.h"
#include <QObject>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<typename T> class QVector;
template<typename Key, typename T> class QMap;
#endif
class QSize;

namespace Phonon
{
	class VideoDataOutputPrivate;
	class VideoFrame;

	/**
	 * \short This class gives you the video data.
	 *
	 * This class implements a special AbstractVideoOutput that gives your
	 * application the video data. If you introduce latencies between receiving
	 * the frameReady signal and displaying the video frame you should call
	 * setDisplayLatency.
	 *
	 * You can also use the video data for further processing (e.g. encoding and
	 * saving to a file).
	 *
	 * The class supports different data formats. See the Format enum for what
	 * is available.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT VideoDataOutput : public QObject, public AbstractVideoOutput
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( VideoDataOutput )
		/**
		 * This property sets the dataformat you'd like to receive.
		 *
		 * The default format is Format_ARGB32.
		 *
		 * \see Format
		 */
		Q_PROPERTY( quint32 format READ format WRITE setFormat )
		//TODO: do we need properties for depth and bpp?

		/**
		 * This property tells the backend how many milliseconds it
		 * takes for the video frame to be displayed after frameReady was
		 * emitted.
		 *
		 * In order to give the backend a chance to perfectly synchronize
		 * audio and video it needs to know how much time you need in order
		 * to display the video frame after it emitted the frameReady
		 * signal. If you render the video to screen immediately, setting the
		 * latency to 0 ms should be good enough.
		 *
		 * If set to -1 the backend may disregard this
		 * object when considering synchronization. Use -1 for cases where
		 * you don't use the video data for displaying it on the screen.
		 *
		 * The latency defaults to 0 ms.
		 */
		//Q_PROPERTY( int displayLatency READ displayLatency WRITE setDisplayLatency )

		/**
		 * This property holds the size of the frame.
		 */
		Q_PROPERTY( QSize frameSize READ frameSize WRITE setFrameSize )

		/**
		 * This property holds the frame rate in Hz. The frame rate tells how
		 * many frameReady signals are emitted per second.
		 */
		Q_PROPERTY( int frameRate READ frameRate WRITE setFrameRate )

		PHONON_HEIR( VideoDataOutput )
		public:
			quint32 format() const;
			void setFormat( quint32 format );

			int frameRate() const;
			void setFrameRate( int );

			//int displayLatency() const;
			//void setDisplayLatency( int milliseconds );

			QSize frameSize() const;
			void setFrameSize( const QSize& size, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio );
			void setFrameSize( int width, int height, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio );

			/**
			 * Tells whether the FOURCC (four character code) is supported by
			 * the backend. If it is supported you can request the backend to
			 * send the video frames in this format.
			 */
			static bool formatSupported( quint32 fourcc );

		Q_SIGNALS:
			/**
			 * Emitted whenever another dataSize number of samples are ready and
			 * format is set to IntegerFormat.
			 *
			 * If format is set to FloatFormat the signal is not emitted at all.
			 *
			 * \param frame An object of class VideoFrame holding the video data
			 * and some additional information.
			 */
			void frameReady( const Phonon::VideoFrame& frame );

			/**
			 * This signal is emitted after the last frameReady signal of a
			 * media is emitted.
			 */
			void endOfMedia();
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_VIDEODATAOUTPUT_H
