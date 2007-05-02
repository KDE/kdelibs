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

#ifndef PHONON_AUDIODEVICEENUMERATOR_H
#define PHONON_AUDIODEVICEENUMERATOR_H

#include "audiodevice.h"
#include <QtCore/QList>
#include <QtCore/QObject>

namespace Phonon
{
    class AudioDeviceEnumeratorPrivate;

    /**
     * \brief Lists available ALSA devices.
     *
     * Simple (singleton) class to list the ALSA devices that are available on the system. A typical
     * use looks like this:
     * \code
     * QList<AudioDevice> deviceList = AudioDeviceEnumerator::availableDevices();
     * foreach (AudioDevice device, deviceList) {
     *     // do something with the device information
     * }
     * \endcode
     *
     * \ingroup Backend
     * \author Matthias Kretz <kretz@kde.org>
     */
    class KAUDIODEVICELIST_EXPORT AudioDeviceEnumerator : public QObject
    {
        friend class AudioDevicePrivate;
        friend class AudioDeviceEnumeratorPrivate;

        Q_OBJECT
        public:
            /**
             * Returns a pointer to an instance of AudioDeviceEnumerator.
             */
            static AudioDeviceEnumerator *self();

            /**
             * Returns a list of the available ALSA playback devices.
             *
             * \see AudioDevice
             */
            static QList<AudioDevice> availablePlaybackDevices();

            /**
             * Returns a list of the available ALSA capture devices.
             *
             * \see AudioDevice
             */
            static QList<AudioDevice> availableCaptureDevices();

        Q_SIGNALS:
            /**
             * Emitted when a new device is available.
             */
            void devicePlugged(const AudioDevice &device);
            /**
             * Emitted when a device disappeared.
             */
            void deviceUnplugged(const AudioDevice &device);

        protected:
            AudioDeviceEnumerator(AudioDeviceEnumeratorPrivate *);
            ~AudioDeviceEnumerator();
            AudioDevice *deviceFor(const QString &internalId);

        private:
            AudioDeviceEnumeratorPrivate *const d;
            Q_PRIVATE_SLOT(d, void _k_deviceAdded(const QString &))
            Q_PRIVATE_SLOT(d, void _k_deviceRemoved(const QString &))
    };
} // namespace Phonon
#endif // PHONON_AUDIODEVICEENUMERATOR_H
