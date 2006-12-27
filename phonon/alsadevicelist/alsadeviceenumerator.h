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

#ifndef PHONON_ALSADEVICEENUMERATOR_H
#define PHONON_ALSADEVICEENUMERATOR_H

#include "alsadevice.h"
#include <QList>
#include <QObject>

namespace Phonon
{
    class AlsaDeviceEnumeratorPrivate;

    /**
     * \brief Lists available ALSA devices.
     *
     * Simple (singleton) class to list the ALSA devices that are available on the system. A typical
     * use looks like this:
     * \code
     * QList<AlsaDevice> deviceList = AlsaDeviceEnumerator::availableDevices();
     * foreach (AlsaDevice device, deviceList) {
     *     // do something with the device information
     * }
     * \endcode
     *
     * \author Matthias Kretz <kretz@kde.org>
     */
    class KALSADEVICELIST_EXPORT AlsaDeviceEnumerator : public QObject
    {
        friend class AlsaDevicePrivate;

        Q_OBJECT
        public:
            /**
             * Returns a pointer to an instance of AlsaDeviceEnumerator.
             */
            static AlsaDeviceEnumerator* self();

            /**
             * Returns a list of the available ALSA devices.
             *
             * \see AlsaDevice
             */
            static QList<AlsaDevice> availableDevices();

        Q_SIGNALS:
            /**
             * Emitted when a new device is available.
             */
            void devicePlugged(const AlsaDevice &device);
            /**
             * Emitted when a device disappeared.
             */
            void deviceUnplugged(const AlsaDevice &device);

        protected:
            AlsaDeviceEnumerator(QObject *parent = 0);
            ~AlsaDeviceEnumerator();
            AlsaDevice *deviceFor(const QString &internalId);

        private:
            static AlsaDeviceEnumerator *s_instance;
            AlsaDeviceEnumeratorPrivate *d;
    };
} // namespace Phonon
#endif // PHONON_ALSADEVICEENUMERATOR_H
