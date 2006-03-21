/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_AUDIOCAPTURESOURCE_H
#define PHONON_AUDIOCAPTURESOURCE_H

#include "namedescriptiontuple.h"

class QString;

namespace Phonon
{
	class AudioSourcePrivate;
	class VideoSource;

/**
 * \short Description for possible AvCapture audio choices.
 *
 * There often are multiple choices of audio sources from the soundcard (or
 * sound system, e.g. soundserver). To let the enduser make a choice this class
 * provides the needed information.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AvCapture
 * \see VideoSource
 */
class PHONON_EXPORT AudioSource : public NameDescriptionTuple
{
	Q_DECLARE_PRIVATE( AudioSource )
	friend class BackendCapabilities;
	friend class AvCapture;
	friend class AvCapturePrivate;
	public:
		/**
		 * Constructs an invalid AudioSource
		 *
		 * \see isValid
		 */
		AudioSource();

		/**
		 * Constructs a copy of \p audioSource.
		 *
		 * \see isValid
		 */
		AudioSource( const AudioSource& audioSource );

		~AudioSource();

		/**
		 * Assigns a copy of the AudioSource \p audioSource to this
		 * AudioSource and returns a reference to it.
		 */
		const AudioSource& operator=( const AudioSource& audioSource );

		/**
		 * Returns \c true if this AudioSource describes the same source
		 * as \p audioSource; otherwise returns \c false.
		 */
		bool operator==( const AudioSource& audioSource ) const;

		/**
		 * Tells whether this audio source is associated with a video
		 * source. For example a DV capture will provide both an audio
		 * and a video signal.
		 *
		 * \return The index of the VideoSource, or \c -1 if
		 * there is none.
		 *
		 * \see associatedVideoSource
		 */
		int indexOfAssociatedVideoSource() const;

		/**
		 * Returns the VideoSource object of the associated video
		 * source.
		 *
		 * \see indexOfAssociatedVideoSource
		 */
		VideoSource associatedVideoSource() const;

		/**
		 * Returns a new AudioSource object that describes the source
		 * with the given \p index.
		 */
		static AudioSource fromIndex( int index );

	protected:
		/**
		 * \internal
		 * Sets the data.
		 */
		AudioSource( int index, const QString& name, const QString& description, int videoIndex = -1 );
};
} //namespace Phonon

#endif // PHONON_CAPTURESOURCE_H
