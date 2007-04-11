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

#include <QtGlobal>
#include <QSharedDataPointer>
#include <QVariant>
#include <QByteArray>
#include <QList>
class QString;

namespace Phonon
{
    class ObjectDescriptionPrivate;

    enum ObjectDescriptionType
    {
        AudioOutputDeviceType,
        AudioCaptureDeviceType,
        VideoOutputDeviceType,
        VideoCaptureDeviceType,
        AudioEffectType,
        VideoEffectType,
        AudioCodecType,
        VideoCodecType,
        ContainerFormatType,
        VisualizationType
    };

/**
 * \short Provides a tuple of enduser visible name and description.
 *
 * Some parts give the enduser choices, e.g. what source to capture audio from.
 * These choices are described by the name and description methods of this class
 * and identified with the id method. Subclasses then define additional
 * information like which audio and video choices belong together.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \see AudioCaptureDevice
 * \see VideoCaptureDevice
 */
template<ObjectDescriptionType T>
class PHONONCORE_EXPORT ObjectDescription
{
    public:
        ObjectDescription(const ObjectDescription<T> &rhs);
        ObjectDescription<T> &operator=(const ObjectDescription<T> &rhs);
        ObjectDescription();
        ~ObjectDescription();

        /**
         * Returns \c true if this ObjectDescription describes the same
         * as \p otherDescription; otherwise returns \c false.
         */
        bool operator==(const ObjectDescription<T> &otherDescription) const;

        /**
         * Returns the name of the capture source.
         *
         * \return A string that should be presented to the user to
         * choose the capture source.
         */
        const QString &name() const;

        /**
         * Returns a description of the capture source. This text should
         * make clear what sound source this is, which is sometimes hard
         * to describe or understand from just the name.
         *
         * \return A string describing the capture source.
         */
        const QString &description() const;

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
         * A unique identifier for this capture source. Used internally
         * to distinguish between the capture sources.
         *
         * \return An integer that uniquely identifies every capture
         * source.
         */
        int index() const;

        /**
         * Returns \c true if the Tuple is valid (index != -1); otherwise returns
         * \c false.
         */
        bool isValid() const;

        /**
         * \internal
         * Returns a new description object that describes the
         * device/effect/codec/...  with the given \p index.
         */
        static ObjectDescription<T> fromIndex(int index);

    protected:
        /**
         * \internal
         * Sets the data.
         */
        ObjectDescription(int index, const QHash<QByteArray, QVariant> &properties);

    private:
        /**
         * \internal
         * The data is implicitly shared.
         */
        QSharedDataPointer<ObjectDescriptionPrivate> d;
};

typedef ObjectDescription<AudioOutputDeviceType> AudioOutputDevice;
typedef ObjectDescription<AudioCaptureDeviceType> AudioCaptureDevice;
typedef ObjectDescription<VideoOutputDeviceType> VideoOutputDevice;
typedef ObjectDescription<VideoCaptureDeviceType> VideoCaptureDevice;
typedef ObjectDescription<AudioEffectType> AudioEffectDescription;
typedef ObjectDescription<VideoEffectType> VideoEffectDescription;
typedef ObjectDescription<AudioCodecType> AudioCodecDescription;
typedef ObjectDescription<VideoCodecType> VideoCodecDescription;
typedef ObjectDescription<ContainerFormatType> ContainerFormatDescription;
typedef ObjectDescription<VisualizationType> VisualizationDescription;

} //namespace Phonon

#endif // PHONON_OBJECTDESCRIPTION_H
// vim: sw=4 ts=4 tw=80
