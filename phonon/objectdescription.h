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

#ifndef PHONON_OBJECTDESCRIPTION_H
#define PHONON_OBJECTDESCRIPTION_H

#include "phonon_export.h"

#include <QtCore/QBool>
#include <QtCore/QSharedData>
#include <QtCore/QVariant>
#include <QtCore/QByteRef>
#include <QtCore/QList>
class QString;

namespace Phonon
{
    class ObjectDescriptionPrivate;

    /**
     * Defines the type of information that is contained in a ObjectDescription
     * object.
     *
     * \ingroup Backend
     */
    enum ObjectDescriptionType
    {
        /**
         * Audio output devices. This can be soundcards (with different drivers), soundservers or
         * other virtual outputs like playback on a different computer on the
         * network.
         *
         * For Hardware devices the backend should use libkaudiodevicelist (\ref
         * AudioDevice and \ref AudioDeviceEnumerator) which will list removable
         * devices even when they are unplugged and provide a unique identifier
         * that can make backends use the same identifiers.
         */
        AudioOutputDeviceType,

        /**
         * Audio capture devices. This can be soundcards (with different drivers), soundservers or
         * other virtual inputs like capture on a different computer on the
         * network.
         *
         * For Hardware devices the backend should use libkaudiodevicelist (\ref
         * AudioDevice and \ref AudioDeviceEnumerator) which will list removable
         * devices even when they are unplugged and provide a unique identifier
         * that can make backends use the same identifiers.
         */
        AudioCaptureDeviceType,
        /**
         * Not used yet.
         */
        VideoOutputDeviceType,
        /**
         * Not used yet.
         */
        VideoCaptureDeviceType,
        /**
         * Lists all audio processing effects the backend supports.
         */
        AudioEffectType,
        /**
         * Lists all video processing effects the backend supports.
         */
        VideoEffectType,
        /**
         * Not used yet.
         */
        AudioCodecType,
        /**
         * Not used yet.
         */
        VideoCodecType,
        /**
         * Not used yet.
         */
        ContainerFormatType,
        /**
         * Not used yet.
         */
        VisualizationType
    };

/** \class ObjectDescriptionBase objectdescription.h Phonon/ObjectDescription
 * \brief Base class for objects describing devices or features of the backend.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see BackendCapabilities
 */
class PHONON_EXPORT ObjectDescriptionBase
{
    public:
        /**
         * Returns \c true if this ObjectDescription describes the same
         * as \p otherDescription; otherwise returns \c false.
         */
        bool operator==(const ObjectDescriptionBase &otherDescription) const;

        /**
         * Returns the name of the capture source.
         *
         * \return A string that should be presented to the user to
         * choose the capture source.
         */
        QString name() const;

        /**
         * Returns a description of the capture source. This text should
         * make clear what sound source this is, which is sometimes hard
         * to describe or understand from just the name.
         *
         * \return A string describing the capture source.
         */
        QString description() const;

        /**
         * Returns a named property.
         *
         * If the property is not set an invalid value is returned.
         *
         * \see propertyNames()
         */
        QVariant property(const char *name) const;

        /**
         * Returns all names that return valid data when property() is called.
         *
         * \see property()
         */
        QList<QByteArray> propertyNames() const;

        /**
         * Returns \c true if the Tuple is valid (index != -1); otherwise returns
         * \c false.
         */
        bool isValid() const;

        /**
         * A unique identifier for this device/. Used internally
         * to distinguish between the devices/.
         *
         * \return An integer that uniquely identifies every device/
         */
        int index() const;

    protected:
        ObjectDescriptionBase(ObjectDescriptionPrivate * = 0);
        ObjectDescriptionBase(const ObjectDescriptionBase &rhs);
        ~ObjectDescriptionBase();

        /**
         * \internal
         * The data is implicitly shared.
         */
        QExplicitlySharedDataPointer<ObjectDescriptionPrivate> d;

    private:
        ObjectDescriptionBase &operator=(const ObjectDescriptionBase &rhs);
};

/** \class ObjectDescription objectdescription.h Phonon/ObjectDescription
 * \short Provides a tuple of enduser visible name and description.
 *
 * Some parts give the enduser choices, e.g. what source to capture audio from.
 * These choices are described by the name and description methods of this class
 * and identified with the id method. Subclasses then define additional
 * information like which audio and video choices belong together.
 *
 * \ingroup Frontend
 * \author Matthias Kretz <kretz@kde.org>
 */
template<ObjectDescriptionType T>
class PHONON_EXPORT ObjectDescription : public ObjectDescriptionBase
{
    public:
        ObjectDescription<T> &operator=(const ObjectDescription<T> &rhs);

        /**
         * Returns a new description object that describes the
         * device/effect/codec/...  with the given \p index.
         */
        static ObjectDescription<T> fromIndex(int index);
};

/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<AudioOutputDeviceType> AudioOutputDevice;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<AudioCaptureDeviceType> AudioCaptureDevice;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<VideoOutputDeviceType> VideoOutputDevice;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<VideoCaptureDeviceType> VideoCaptureDevice;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<AudioEffectType> AudioEffectDescription;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<VideoEffectType> VideoEffectDescription;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<AudioCodecType> AudioCodecDescription;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<VideoCodecType> VideoCodecDescription;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<ContainerFormatType> ContainerFormatDescription;
/**
 * \ingroup BackendInformation
 */
typedef ObjectDescription<VisualizationType> VisualizationDescription;

} //namespace Phonon

#endif // PHONON_OBJECTDESCRIPTION_H
