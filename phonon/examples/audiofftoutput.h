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
#ifndef Phonon_AUDIOFFTOUTPUT_H
#define Phonon_AUDIOFFTOUTPUT_H

#include <kdelibs_export.h>

template<class T> class QVector;

namespace Phonon
{
	class AudioFftOutputPrivate;

	/**
	 * @short Fourier transforms the audio data for visualizations.
	 *
	 * This class is useful to easily get the fourier transformed audio data,
	 * i.e. the frequency spectrum. The spectrum can be used for visualizations.
	 *
	 * @author Matthias Kretz <kretz@kde.org>
	 */
	class PHONON_EXPORT AudioFftOutput : public AbstractAudioOutput
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( AudioFftOutput )
		Q_PROPERTY( int rate READ rate WRITE setRate )
		Q_PROPERTY( int bandwidth READ bandwidth WRITE setBandwidth )
		PHONON_HEIR( AudioFftOutput )

		public:
			/**
			 * Returns the last available frequency spectrum.
			 *
			 * @return The return value is a vector of floats. The first value
			 * in the vector is the fraction of the spectrium with the frequencies
			 * 0 Hz to bandwidth Hz, the second bandwidth Hz to 2*bandwidth Hz
			 * and so forth.
			 *
			 * @see setBandwidth
			 * @see fourierTransformedData
			 */
			QVector<float> fourierTransformedData() const;

			/**
			 * Returns the bandwidth of the values in the spectrum.
			 *
			 * @see setBandwidth
			 */
			int bandwidth() const;

			/**
			 * Returns how many fourier transformations are calculated per second
			 *
			 * @see setRate
			 */
			int rate() const;

		public Q_SLOTS:
			/**
			 * Sets the bandwidth of the values in the frequency spectrum. The
			 * sampling frequency has to be a multiple of the bandwidth. If the
			 * passed value does not satisfy that restricition the next smaller
			 * possible bandwidth is used.
			 *
			 * @param newBandwidth A value in Hz. Numbers close to 1 are normally
			 * overkill as you'd get thousands of floats (depending of the
			 * sampling frequency used underneath) from fourierTransformedData.
			 *
			 * @return The method returns the new bandwidth which is smaller or
			 * equal to @p newBandwidth.
			 *
			 * @see bandwidth
			 */
			int setBandwidth( int newBandwidth );

			/**
			 * Sets how many fourier transformations should be calculated per
			 * second.
			 *
			 * @param newRate The rate in Hz.
			 *
			 * @see rate
			 */
			void setRate( int newRate );

		Q_SIGNALS:
			/**
			 * Emitted whenever a fourier transformation has been calculated.
			 *
			 * @param spectrum The frequency spectrum.
			 */
			void fourierTransformedData( const QVector<float>& spectrum );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // Phonon_AUDIOFFTOUTPUT_H
