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
#ifndef PHONON_AVCAPTURE_H
#define PHONON_AVCAPTURE_H

#include "export.h"
#include "../abstractmediaproducer.h"
#include "../phonondefs.h"
#include "../objectdescription.h"

class QString;
class QStringList;

namespace Phonon
{
namespace Experimental
{

	class AvCapturePrivate;

	/**
	 * @short Media data from a soundcard, soundserver, camera or any other
	 * hardware device supported by the backend.
	 *
	 * This class gives you access to the capture capabilities of the backend.
	 * There might be more than only one possible capture source, for audio, as
	 * well as for video. The list of available sources is available through
	 * BackendCapabilities::availableAudioCaptureDevices and
	 * BackendCapabilities::availableVideoCaptureDevices.
	 *
     * @ingroup Recording
	 * @author Matthias Kretz <kretz@kde.org>
	 * @see BackendCapabilities::availableAudioCaptureDevices
	 * @see BackendCapabilities::availableVideoCaptureDevices
	 */
	class PHONONEXPERIMENTAL_EXPORT AvCapture : public AbstractMediaProducer
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( AvCapture )
		PHONON_HEIR( AvCapture )
		Q_PROPERTY( AudioCaptureDevice audioCaptureDevice READ audioCaptureDevice WRITE setAudioCaptureDevice )
		Q_PROPERTY( VideoCaptureDevice videoCaptureDevice READ videoCaptureDevice WRITE setVideoCaptureDevice )
		public:
			/**
			 * Returns the currently used capture source for the audio signal.
			 *
			 * @see AudioCaptureDevice
			 * @see setAudioCaptureDevice( const AudioCaptureDevice& )
			 * @see setAudioCaptureDevice( int )
			 */
			AudioCaptureDevice audioCaptureDevice() const;

			/**
			 * Returns the currently used capture source for the video signal.
			 *
			 * @see VideoCaptureDevice
			 * @see setVideoCaptureDevice( const VideoCaptureDevice& )
			 * @see setVideoCaptureDevice( int )
			 */
			VideoCaptureDevice videoCaptureDevice() const;

		public Q_SLOTS:
			/**
			 * Sets the audio capture source to use.
			 *
			 * @param source An object of class AudioCaptureDevice. A list of
			 * available objects can be queried from
			 * BackendCapabilities::availableAudioCaptureDevices.
			 *
			 * @see audioCaptureDevice
			 * @see setAudioCaptureDevice( int )
			 */
			void setAudioCaptureDevice( const AudioCaptureDevice& source );

			/**
			 * Sets the capture source to use.
			 *
			 * @param sourceIndex An index corresponding an object of class
			 * AudioCaptureDevice. A list of available objects can be queried from
			 * BackendCapabilities::availableAudioCaptureDevices.
			 *
			 * @see audioCaptureDevice
			 * @see setAudioCaptureDevice( const AudioCaptureDevice& )
			 */
			void setAudioCaptureDevice( int sourceIndex );

			/**
			 * Sets the video capture source to use.
			 *
			 * @param source An object of class VideoCaptureDevice. A list of
			 * available objects can be queried from
			 * BackendCapabilities::availableVideoCaptureDevices.
			 *
			 * @see videoCaptureDevice
			 * @see setVideoCaptureDevice( int )
			 */
			void setVideoCaptureDevice( const VideoCaptureDevice& source );

			/**
			 * Sets the capture source to use.
			 *
			 * @param sourceIndex An index corresponding an object of class
			 * VideoCaptureDevice. A list of available objects can be queried from
			 * BackendCapabilities::availableVideoCaptureDevices.
			 *
			 * @see videoCaptureDevice
			 * @see setVideoCaptureDevice( const VideoCaptureDevice& )
			 */
			void setVideoCaptureDevice( int sourceIndex );
	};

} // namespace Experimental
} // namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_AVCAPTURE_H
