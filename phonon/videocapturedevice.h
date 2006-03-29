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

#ifndef PHONON_VIDEOCAPTUREDEVICE_H
#define PHONON_VIDEOCAPTUREDEVICE_H

#include "namedescriptiontuple.h"

class QString;

namespace Phonon
{
	class VideoCaptureDevicePrivate;
	class AudioCaptureDevice;

/**
 * \short Description for possible AvCapture video choices.
 *
 * There often are multiple choices of video sources from the soundcard (or
 * sound system, e.g. soundserver). To let the enduser make a choice this class
 * provides the needed information.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AvCapture
 * \see AudioCaptureDevice
 */
class PHONONCORE_EXPORT VideoCaptureDevice : public NameDescriptionTuple
{
	Q_DECLARE_PRIVATE( VideoCaptureDevice )
	public:
		/**
		 * Constructs an invalid VideoCaptureDevice
		 *
		 * \see isValid
		 */
		VideoCaptureDevice();

		/**
		 * Constructs a copy of \p videoCaptureDevice.
		 *
		 * \see isValid
		 */
		VideoCaptureDevice( const VideoCaptureDevice& videoCaptureDevice );

		~VideoCaptureDevice();

		/**
		 * Assigns a copy of the VideoCaptureDevice \p videoCaptureDevice to this
		 * VideoCaptureDevice and returns a reference to it.
		 */
		VideoCaptureDevice& operator=( const VideoCaptureDevice& videoCaptureDevice );

		/**
		 * Returns \c true if this VideoCaptureDevice describes the same source
		 * as \p videoCaptureDevice; otherwise returns \c false.
		 */
		bool operator==( const VideoCaptureDevice& videoCaptureDevice ) const;

		/**
		 * Tells whether this video source is associated with a audio
		 * source. For example a DV capture will provide both an audio
		 * and a video signal.
		 *
		 * \return The index of the AudioCaptureDevice, or \c -1 if
		 * there is none.
		 */
		int indexOfAssociatedAudioCaptureDevice() const;

		/**
		 * Returns the AudioCaptureDevice object of the associated audio
		 * source.
		 *
		 * \see indexOfAssociatedAudioCaptureDevice
		 */
		AudioCaptureDevice associatedAudioCaptureDevice() const;

		/**
		 * Returns a new VideoCaptureDevice object that describes the source
		 * with the given \p index.
		 */
		static VideoCaptureDevice fromIndex( int index );

	protected:
		/**
		 * \internal
		 * Sets the data.
		 */
		VideoCaptureDevice( int index, const QString& name, const QString& description, int audioIndex = -1 );
};
} //namespace Phonon

#endif // PHONON_VIDEOCAPTUREDEVICE_H
