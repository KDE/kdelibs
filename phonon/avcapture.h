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

#include "abstractmediaproducer.h"
#include "phonondefs.h"

class QString;
class QStringList;

namespace Phonon
{
	class AvCapturePrivate;
	class AudioSource;
	class VideoSource;
	namespace Ifaces
	{
		class AvCapture;
	}

	/**
	 * @short Media data from a soundcard, soundserver, camera or any other
	 * hardware device supported by the backend.
	 *
	 * This class gives you access to the capture capabilities of the backend.
	 * There might be more than only one possible capture source, for audio, as
	 * well as for video. The list of available sources is available through
	 * BackendCapabilities::availableAudioSources and
	 * BackendCapabilities::availableVideoSources.
	 *
	 * @author Matthias Kretz <kretz@kde.org>
	 * @see BackendCapabilities::availableAudioSources
	 * @see BackendCapabilities::availableVideoSources
	 */
	class PHONON_EXPORT AvCapture : public AbstractMediaProducer
	{
		Q_OBJECT
		K_DECLARE_PRIVATE( AvCapture )
		PHONON_HEIR( AvCapture )
		public:
			/**
			 * Returns the currently used capture source for the audio signal.
			 *
			 * @see AudioSource
			 * @see setAudioSource( const AudioSource& )
			 * @see setAudioSource( int )
			 */
			const AudioSource& audioSource() const;

			/**
			 * Returns the currently used capture source for the video signal.
			 *
			 * @see VideoSource
			 * @see setVideoSource( const VideoSource& )
			 * @see setVideoSource( int )
			 */
			const VideoSource& videoSource() const;

		public Q_SLOTS:
			/**
			 * Sets the audio capture source to use.
			 *
			 * @param source An object of class AudioSource. A list of
			 * available objects can be queried from
			 * BackendCapabilities::availableAudioSources.
			 *
			 * @see audioSource
			 * @see setAudioSource( int )
			 */
			void setAudioSource( const AudioSource& source );

			/**
			 * Sets the capture source to use.
			 *
			 * @param sourceIndex An index corresponding an object of class
			 * AudioSource. A list of available objects can be queried from
			 * BackendCapabilities::availableAudioSources.
			 *
			 * @see audioSource
			 * @see setAudioSource( const AudioSource& )
			 */
			void setAudioSource( int sourceIndex );

			/**
			 * Sets the video capture source to use.
			 *
			 * @param source An object of class VideoSource. A list of
			 * available objects can be queried from
			 * BackendCapabilities::availableVideoSources.
			 *
			 * @see videoSource
			 * @see setVideoSource( int )
			 */
			void setVideoSource( const VideoSource& source );

			/**
			 * Sets the capture source to use.
			 *
			 * @param sourceIndex An index corresponding an object of class
			 * VideoSource. A list of available objects can be queried from
			 * BackendCapabilities::availableVideoSources.
			 *
			 * @see videoSource
			 * @see setVideoSource( const VideoSource& )
			 */
			void setVideoSource( int sourceIndex );
	};
} //namespace Phonon

// vim: sw=4 ts=4 tw=80 noet
#endif // PHONON_AVCAPTURE_H
