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

#ifndef PHONON_VIDEOCAPTURESOURCE_H
#define PHONON_VIDEOCAPTURESOURCE_H

#include "namedescriptiontuple.h"

class QString;

namespace Phonon
{
	class VideoSourcePrivate;
	class AudioSource;

/**
 * \short Description for possible AvCapture video choices.
 *
 * There often are multiple choices of video sources from the soundcard (or
 * sound system, e.g. soundserver). To let the enduser make a choice this class
 * provides the needed information.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AvCapture
 * \see AudioSource
 */
class PHONONCORE_EXPORT VideoSource : public NameDescriptionTuple
{
	Q_DECLARE_PRIVATE( VideoSource )
	friend class BackendCapabilities;
	friend class AvCapture;
	public:
		/**
		 * Constructs an invalid VideoSource
		 *
		 * \see isValid
		 */
		VideoSource();

		/**
		 * Constructs a copy of \p videoSource.
		 *
		 * \see isValid
		 */
		VideoSource( const VideoSource& videoSource );

		~VideoSource();

		/**
		 * Assigns a copy of the VideoSource \p videoSource to this
		 * VideoSource and returns a reference to it.
		 */
		const VideoSource& operator=( const VideoSource& videoSource );

		/**
		 * Returns \c true if this VideoSource describes the same source
		 * as \p videoSource; otherwise returns \c false.
		 */
		bool operator==( const VideoSource& videoSource ) const;

		/**
		 * Tells whether this video source is associated with a audio
		 * source. For example a DV capture will provide both an audio
		 * and a video signal.
		 *
		 * \return The index of the AudioSource, or \c -1 if
		 * there is none.
		 */
		int indexOfAssociatedAudioSource() const;

		/**
		 * Returns the AudioSource object of the associated audio
		 * source.
		 *
		 * \see indexOfAssociatedAudioSource
		 */
		AudioSource associatedAudioSource() const;

		/**
		 * Returns a new VideoSource object that describes the source
		 * with the given \p index.
		 */
		static VideoSource fromIndex( int index );

	protected:
		/**
		 * \internal
		 * Sets the data.
		 */
		VideoSource( int index, const QString& name, const QString& description, int audioIndex = -1 );
};
} //namespace Phonon

#endif // PHONON_CAPTURESOURCE_H
