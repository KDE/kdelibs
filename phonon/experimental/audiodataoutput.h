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
#ifndef Phonon_AUDIODATAOUTPUT_H
#define Phonon_AUDIODATAOUTPUT_H

#include "export.h"
#include "../abstractaudiooutput.h"
#include "../phonondefs.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
template<typename T> class QVector;
template<typename Key, typename T> class QMap;
#endif

namespace Phonon
{
namespace Experimental
{
	class AudioDataOutputPrivate;

	/**
	 * \short This class gives you the audio data (for visualizations).
	 *
	 * This class implements a special AbstractAudioOutput that gives your
	 * application the audio data. Don't expect realtime performance. But
	 * the latencies should be low enough to use the audio data for
	 * visualizations. You can also use the audio data for further processing
	 * (e.g. encoding and saving to a file).
	 *
	 * The class supports different data formats. One of the most common formats
	 * is to read vectors of integers (which will only use 16 Bit), but you can
	 * also request floats which some backends use internally.
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 */
	class PHONONEXPERIMENTAL_EXPORT AudioDataOutput : public AbstractAudioOutput
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( AudioDataOutput )
		Q_ENUMS( Channel Format )
		Q_PROPERTY( Format format READ format WRITE setFormat )
		Q_PROPERTY( int dataSize READ dataSize WRITE setDataSize )
		PHONON_HEIR( AudioDataOutput )
		public:
			/**
			 * Specifies the channel the audio data belongs to.
			 */
			enum Channel
			{
				LeftChannel,
				RightChannel,
				CenterChannel,
				LeftSurroundChannel,
				RightSurroundChannel,
				SubwooferChannel
			};

			/**
			 * Used for telling the object whether you want 16 bit Integers or
			 * 32 bit floats.
			 *
			 * \see requestFormat
			 */
			enum Format
			{
				/**
				 * Requests 16 bit signed integers.
				 *
				 * \see dataReady( const QVector<qint16>& )
				 */
				IntegerFormat = 1,
				/**
				 * Requests 32 bit floating point: signed, zero centered, and
				 * normalized to the unit value (-1.0 to 1.0).
				 *
				 * \see dataReady( const QVector<float>& )
				 */
				FloatFormat = 2
			};

			/**
			 * Returns the currently used format.
			 *
			 * \see setFormat
			 */
			Format format() const;

			/**
			 * Returns the currently used number of samples passed through
			 * the signal.
			 *
			 * \see setDataSize
			 */
			int dataSize() const;

			/**
			 * Returns the sample rate in Hz. Common sample rates are 44100 Hz
			 * and 48000 Hz. AudioDataOutput will not do any sample rate
			 * conversion for you. If you need to convert the sample rate you
			 * might want to take a look at libsamplerate. For visualizations it
			 * is often enough to do simple interpolation or even drop/duplicate
			 * samples.
			 *
			 * \return The sample rate as reported by the backend. If the
			 * backend is unavailable -1 is returned.
			 */
			int sampleRate() const;

		public Q_SLOTS:
			/**
			 * Requests the dataformat you'd like to receive. Only one of the
			 * signals of this class will be emitted when new data is ready.
			 *
			 * The default format is IntegerFormat.
			 *
			 * \see format()
			 */
			void setFormat( Format format );

			/**
			 * Sets the number of samples to be passed in one signal emission.
			 *
			 * Defaults to 512 samples per emitted signal.
			 *
			 * \param size the number of samples
			 */
			void setDataSize( int size );

		Q_SIGNALS:
			/**
			 * Emitted whenever another dataSize number of samples are ready and
			 * format is set to IntegerFormat.
			 *
			 * If format is set to FloatFormat the signal is not emitted at all.
			 *
			 * \param data A mapping of Channel to a vector holding the audio data.
			 */
			void dataReady( const QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<qint16> >& data );

			/**
			 * Emitted whenever another dataSize number of samples are ready and
			 * format is set to FloatFormat.
			 *
			 * If format is set to IntegerFormat the signal is not emitted at all.
			 *
			 * \param data A mapping of Channel to a vector holding the audio data.
			 */
			void dataReady( const QMap<Phonon::Experimental::AudioDataOutput::Channel, QVector<float> >& data );

			/**
			 * This signal is emitted before the last dataReady signal of a
			 * media is emitted.
			 * 
			 * If, for example, the playback of a media file has finished and the
			 * last audio data of that file is going to be passed with the next
			 * dataReady signal, and only the 28 first samples of the data
			 * vector are from that media file endOfMedia will be emitted right
			 * before dataReady with \p remainingSamples = 28.
			 *
			 * \param remainingSamples The number of samples in the next
			 * dataReady vector that belong to the media that was playing to
			 * this point.
			 */
			void endOfMedia( int remainingSamples );
	};
} // namespace Experimental
} // namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_AUDIODATAOUTPUT_H
