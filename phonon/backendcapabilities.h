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

#ifndef Phonon_BACKENDCAPABILITIES_H
#define Phonon_BACKENDCAPABILITIES_H

#include <QObject>

#include <kdelibs_export.h>
#include <kstaticdeleter.h>
#include "objectdescription.h"

template<class T> class QList;
class QStringList;

namespace Phonon
{

/**
 * Singleton class describing the capabilities of the Backend.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONCORE_EXPORT BackendCapabilities : public QObject
{
	friend void ::KStaticDeleter<BackendCapabilities>::destructObject();

	Q_OBJECT
	public:
		/**
		 * Use this function to get the QObject pointer for connecting the
		 * capabilitiesChanged signal.
		 *
		 * \return a pointer to the BackendCapabilities instance. If no instance existed before a new
		 * one is created.
		 */
		static BackendCapabilities* self();

		/**
		 * Tells whether the backend is audio only or can handle video files
		 * and display of videos.
		 *
		 * If the backend does not support video the classes
		 * VideoPath, VideoEffect and all AbstractVideoOutput subclasses won't
		 * do anything.
		 */
		static bool supportsVideo();

		/**
		 * Tells whether the backend implements the OSD interfaces.
		 *
		 * \todo there's no interfaces for it at this point
		 */
		static bool supportsOSD();

		/**
		 * Tells whether the backend supports subtitle rendering on the
		 * video output.
		 *
		 * \todo there's no interfaces for it at this point
		 */
		static bool supportsSubtitles();

		/**
		 * Returns a list of mime types that the Backend can decode.
		 *
		 * \see isMimeTypeKnown()
		 */
		static QStringList knownMimeTypes();

		/**
		 * Often all you want to know is whether one given MIME type can be
		 * decoded by the backend. Use this method in favor of knownMimeTypes()
		 * as it can give you a negative answer without having a backend loaded.
		 *
		 * \see knownMimeTypes();
		 */
		static bool isMimeTypeKnown( const QString& mimeType );

#if 0
		/**
		 * Returns the audio output devices the backend reports as usable. That
		 * doesn't have to say the device will certainly work as the backend
		 * might not be able to open the device if it's blocked by another
		 * application. But at least the device is physically available.
		 *
		 * \return A list of AudioOutputDevice objects that give a name and
		 * description for every supported audio output device.
		 * 
		 * \see knownAudioOutputDevices
		 */
		static QList<AudioOutputDevice> usableAudioOutputDevices();

		/**
		 * Returns the audio output devices the backend has ever reported as
		 * usable.
		 *
		 * \return A list of AudioOutputDevice objects that give a name and
		 * description for every supported audio output device.
		 *
		 * \see usableAudioOutputDevices
		 */
		static QList<AudioOutputDevice> knownAudioOutputDevices();
#endif
		/**
		 * Returns the audio output devices the backend supports.
		 *
		 * \return A list of AudioOutputDevice objects that give a name and
		 * description for every supported audio output device.
		 */
		static QList<AudioOutputDevice> availableAudioOutputDevices();

		/**
		 * Returns the audio capture devices the backend supports.
		 *
		 * \return A list of AudioCaptureDevice objects that give a name and
		 * description for every supported audio capture device.
		 */
		static QList<AudioCaptureDevice> availableAudioCaptureDevices();

		/**
		 * Returns the video output devices the backend supports.
		 *
		 * \return A list of VideoOutputDevice objects that give a name and
		 * description for every supported video output device.
		 */
		static QList<VideoOutputDevice> availableVideoOutputDevices();

		/**
		 * Returns the video capture devices the backend supports.
		 *
		 * \return A list of VideoCaptureDevice objects that give a name and
		 * description for every supported video capture device.
		 */
		static QList<VideoCaptureDevice> availableVideoCaptureDevices();

		/**
		 * Returns the visualization effects the backend supports.
		 *
		 * \return A list of VisualizationEffect objects that give a name and
		 * description for every supported visualization effect.
		 */
		static QList<VisualizationDescription> availableVisualizations();

		/**
		 * Returns descriptions for the audio effects the backend supports.
		 *
		 * \return A list of AudioEffectDescription objects that give a name and
		 * description for every supported audio effect.
		 */
		static QList<AudioEffectDescription> availableAudioEffects();

		/**
		 * Returns descriptions for the video effects the backend supports.
		 *
		 * \return A list of VideoEffectDescription objects that give a name and
		 * description for every supported video effect.
		 */
		static QList<VideoEffectDescription> availableVideoEffects();

		/**
		 * Returns descriptions for the audio codecs the backend supports.
		 *
		 * \return A list of AudioCodec objects that give a name and
		 * description for every supported audio codec.
		 */
		static QList<AudioCodecDescription> availableAudioCodecs();

		/**
		 * Returns descriptions for the video codecs the backend supports.
		 *
		 * \return A list of VideoCodec objects that give a name and
		 * description for every supported video codec.
		 */
		static QList<VideoCodecDescription> availableVideoCodecs();

		/**
		 * Returns descriptions for the container formats the backend supports.
		 *
		 * \return A list of ContainerFormat objects that give a name and
		 * description for every supported container format.
		 */
		static QList<ContainerFormatDescription> availableContainerFormats();

	Q_SIGNALS:
		/**
		 * This signal is emitted if the capabilities have changed. This can
		 * happen if the user has requested a backend change.
		 *
		 * To connect to this signal use the self function:
		 * \code
		 * QObject::connect( BackendCapabilities::self(), SIGNAL(capabilitiesChanged()), ...
		 * \endcode
		 */
		void capabilitiesChanged();

	protected:
		BackendCapabilities();
		~BackendCapabilities();

	private Q_SLOTS:
		void _k_slotBackendChanged();

	private:
		static BackendCapabilities* m_self;
		class Private;
		Private* d;
};
} // namespace Phonon

#endif // Phonon_BACKENDCAPABILITIES_H
// vim: sw=4 ts=4 tw=80
