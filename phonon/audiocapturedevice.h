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

#ifndef PHONON_AUDIOCAPTUREDEVICE_H
#define PHONON_AUDIOCAPTUREDEVICE_H

#include "namedescriptiontuple.h"

class QString;

namespace Phonon
{
	class AudioCaptureDevicePrivate;
	class VideoCaptureDevice;

/**
 * \short Description for possible AvCapture audio choices.
 *
 * There often are multiple choices of audio sources from the soundcard (or
 * sound system, e.g. soundserver). To let the enduser make a choice this class
 * provides the needed information.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AvCapture
 * \see VideoCaptureDevice
 */
class PHONONCORE_EXPORT AudioCaptureDevice : public NameDescriptionTuple
{
	Q_DECLARE_PRIVATE( AudioCaptureDevice )
	public:
		/**
		 * Constructs an invalid AudioCaptureDevice
		 *
		 * \see isValid
		 */
		AudioCaptureDevice();

		/**
		 * Constructs a copy of \p audioCaptureDevice.
		 *
		 * \see isValid
		 */
		AudioCaptureDevice( const AudioCaptureDevice& audioCaptureDevice );

		~AudioCaptureDevice();

		/**
		 * Assigns a copy of the AudioCaptureDevice \p audioCaptureDevice to this
		 * AudioCaptureDevice and returns a reference to it.
		 */
		AudioCaptureDevice& operator=( const AudioCaptureDevice& audioCaptureDevice );

		/**
		 * Returns \c true if this AudioCaptureDevice describes the same source
		 * as \p audioCaptureDevice; otherwise returns \c false.
		 */
		bool operator==( const AudioCaptureDevice& audioCaptureDevice ) const;

		/**
		 * Tells whether this audio source is associated with a video
		 * source. For example a DV capture will provide both an audio
		 * and a video signal.
		 *
		 * \return The index of the VideoCaptureDevice, or \c -1 if
		 * there is none.
		 *
		 * \see associatedVideoCaptureDevice
		 */
		int indexOfAssociatedVideoCaptureDevice() const;

		/**
		 * Returns the VideoCaptureDevice object of the associated video
		 * source.
		 *
		 * \see indexOfAssociatedVideoCaptureDevice
		 */
		VideoCaptureDevice associatedVideoCaptureDevice() const;

		/**
		 * Returns a new AudioCaptureDevice object that describes the source
		 * with the given \p index.
		 */
		static AudioCaptureDevice fromIndex( int index );

	protected:
		/**
		 * \internal
		 * Sets the data.
		 */
		AudioCaptureDevice( int index, const QString& name, const QString& description, int videoIndex = -1 );
};
} //namespace Phonon

#endif // PHONON_AUDIOCAPTUREDEVICE_H
