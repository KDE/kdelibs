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

namespace Phonon
{
	class VideoDataOutputPrivate;
	class VideoFrame;
	namespace Ifaces
	{
		class VideoDataOutput;
	}

	/**
	 * \short This class gives you the video data (for visualizations).
	 *
	 * This class implements a special AbstractVideoOutput that gives your
	 * application the video data. Don't expect realtime performance. But
	 * the latencies should be low enough to use the video data for
	 * visualizations. You can also use the video data for further processing
	 * (e.g. encoding and saving to a file).
	 *
	 * The class supports different data formats. One of the most common formats
	 * is to read vectors of integers (which will only use 16 Bit), but you can
	 * also request floats which some backends use internally.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONCORE_EXPORT VideoDataOutput : public QObject, public AbstractVideoOutput
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( VideoDataOutput )
		Q_ENUMS( Format )
		Q_PROPERTY( Format format READ format WRITE setFormat )
		PHONON_HEIR( VideoDataOutput )
		public:
			/**
			 * Specifies the format for the frames in the video stream.
			 */
			enum Format
			{
				Format_ARGB32,
				Format_RGB32,
				Format_RGB24,
				Format_YV12,
				Format_I420
			};

			/**
			 * Returns the currently used format.
			 *
			 * \see setFormat
			 */
			Format format() const;

			/**
			 * Returns the frame rate in Hz.
			 *
			 * VideoDataOutput will not do any frame rate conversion.
			 *
			 * \return The frame rate as reported by the backend. If the backend
			 * is unavailable -1 is returned.
			 */
			int frameRate() const;

		public Q_SLOTS:
			/**
			 * Requests the dataformat you'd like to receive. Only one of the
			 * signals of this class will be emitted when new data is ready.
			 *
			 * The default format is .
			 *
			 * \see format()
			 */
			//TODO document default
			void setFormat( Phonon::VideoDataOutput::Format format );

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

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_VIDEODATAOUTPUT_H
