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
#include "objectdescription.h"
#include <kglobal.h>

template<class T> class QList;
class QStringList;

namespace Phonon
{

/**
 * Singleton class describing the capabilities of the Backend.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
namespace BackendCapabilities
{
    /**
     * Tells whether the backend is audio only or can handle video files
     * and display of videos.
     *
     * If the backend does not support video the classes
     * VideoPath, VideoEffect and all AbstractVideoOutput subclasses won't
     * do anything.
     */
    bool supportsVideo() PHONONCORE_EXPORT;

    /**
     * Tells whether the backend implements the OSD interfaces.
     *
     * \todo there's no interfaces for it at this point
     */
    bool supportsOSD() PHONONCORE_EXPORT;

    /**
     * Tells whether the backend supports subtitle rendering on the
     * video output.
     *
     * \todo there's no interfaces for it at this point
     */
    bool supportsSubtitles() PHONONCORE_EXPORT;

    /**
     * Returns a list of mime types that the Backend can decode.
     *
     * \see isMimeTypeKnown()
     */
    QStringList knownMimeTypes() PHONONCORE_EXPORT;

    /**
     * Often all you want to know is whether one given MIME type can be
     * decoded by the backend. Use this method in favor of knownMimeTypes()
     * as it can give you a negative answer without having a backend loaded.
     *
     * \see knownMimeTypes();
     */
    bool isMimeTypeKnown( const QString& mimeType ) PHONONCORE_EXPORT;

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
    QList<AudioOutputDevice> usableAudioOutputDevices() PHONONCORE_EXPORT;

    /**
     * Returns the audio output devices the backend has ever reported as
     * usable.
     *
     * \return A list of AudioOutputDevice objects that give a name and
     * description for every supported audio output device.
     *
     * \see usableAudioOutputDevices
     */
    QList<AudioOutputDevice> knownAudioOutputDevices() PHONONCORE_EXPORT;
#endif
    /**
     * Returns the audio output devices the backend supports.
     *
     * \return A list of AudioOutputDevice objects that give a name and
     * description for every supported audio output device.
     */
    QList<AudioOutputDevice> availableAudioOutputDevices() PHONONCORE_EXPORT;

    /**
     * Returns the audio capture devices the backend supports.
     *
     * \return A list of AudioCaptureDevice objects that give a name and
     * description for every supported audio capture device.
     */
    QList<AudioCaptureDevice> availableAudioCaptureDevices() PHONONCORE_EXPORT;

    /**
     * Returns the video output devices the backend supports.
     *
     * \return A list of VideoOutputDevice objects that give a name and
     * description for every supported video output device.
     */
    QList<VideoOutputDevice> availableVideoOutputDevices() PHONONCORE_EXPORT;

    /**
     * Returns the video capture devices the backend supports.
     *
     * \return A list of VideoCaptureDevice objects that give a name and
     * description for every supported video capture device.
     */
    QList<VideoCaptureDevice> availableVideoCaptureDevices() PHONONCORE_EXPORT;

    /**
     * Returns the visualization effects the backend supports.
     *
     * \return A list of VisualizationEffect objects that give a name and
     * description for every supported visualization effect.
     */
    QList<VisualizationDescription> availableVisualizations() PHONONCORE_EXPORT;

    /**
     * Returns descriptions for the audio effects the backend supports.
     *
     * \return A list of AudioEffectDescription objects that give a name and
     * description for every supported audio effect.
     */
    QList<AudioEffectDescription> availableAudioEffects() PHONONCORE_EXPORT;

    /**
     * Returns descriptions for the video effects the backend supports.
     *
     * \return A list of VideoEffectDescription objects that give a name and
     * description for every supported video effect.
     */
    QList<VideoEffectDescription> availableVideoEffects() PHONONCORE_EXPORT;

    /**
     * Returns descriptions for the audio codecs the backend supports.
     *
     * \return A list of AudioCodec objects that give a name and
     * description for every supported audio codec.
     */
    QList<AudioCodecDescription> availableAudioCodecs() PHONONCORE_EXPORT;

    /**
     * Returns descriptions for the video codecs the backend supports.
     *
     * \return A list of VideoCodec objects that give a name and
     * description for every supported video codec.
     */
    QList<VideoCodecDescription> availableVideoCodecs() PHONONCORE_EXPORT;

    /**
     * Returns descriptions for the container formats the backend supports.
     *
     * \return A list of ContainerFormat objects that give a name and
     * description for every supported container format.
     */
    QList<ContainerFormatDescription> availableContainerFormats() PHONONCORE_EXPORT;

    /**
     * Use this function to get a QObject pointer to connect to the capabilitiesChanged signal.
     *
     * \return a pointer to a QObject.
     *
     * The capabilitiesChanged signal is emitted if the capabilities have changed. This can
     * happen if the user has requested a backend change.
     *
     * To connect to this signal do the following:
     * \code
     * QObject::connect(BackendCapabilities::notifier(), SIGNAL(capabilitiesChanged()), ...
     * \endcode
     */
    QObject *notifier() PHONONCORE_EXPORT;
} // namespace BackendCapabilities
} // namespace Phonon

#endif // Phonon_BACKENDCAPABILITIES_H
// vim: sw=4 ts=4 tw=80
