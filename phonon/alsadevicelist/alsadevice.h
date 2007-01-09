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

#ifndef PHONON_ALSADEVICE_H
#define PHONON_ALSADEVICE_H

#include <QSharedDataPointer>
#include <kdelibs_export.h>
#include <kconfig.h>
class QString;
class QStringList;

namespace Solid
{
    class AudioHw;
} // namespace Solid

namespace Phonon
{
    class AlsaDevicePrivate;

    /**
     * \brief Information about a soundcard of the computer.
     *
     * This class encapsulates some useful information about a soundcard of the computer. Don't
     * instanciate this class in your code, use AlsaDeviceEnumerator::availableDevices().
     *
     * \see AlsaDeviceEnumerator
     * \author Matthias Kretz <kretz@kde.org>
     */
    class KALSADEVICELIST_EXPORT AlsaDevice
    {
        friend class AlsaDevicePrivate;
        friend class AlsaDeviceEnumerator;
        friend class AlsaDeviceEnumeratorPrivate;

        public:
            /**
             * \internal
             * Creates an invalid and empty instance.
             */
            AlsaDevice();
            /**
             * Copy constructor. The data is implicitly shared, so copying is cheap.
             */
            AlsaDevice(const AlsaDevice &rhs);
            /**
             * Destroys the object.
             */
            ~AlsaDevice();

            /**
             * Assignment operator. The data is implicitly shared, so copying is cheap.
             */
            AlsaDevice& operator=(const AlsaDevice &rhs);
            /**
             * Equality operator.
             */
            bool operator==(const AlsaDevice &rhs) const;
            /**
             * Inequality operator.
             */
            bool operator!=(const AlsaDevice &rhs) const { return !operator==(rhs); }

            /**
             * Returns the name of the soundcard. This string
             * should be shown to the user to select from multiple soundcards.
             */
            QString cardName() const;

            /**
             * Returns a list of device identifiers that your code can use in a snd_pcm_open call.
             * If the code wants to open the soundcard identified by this object it should try all
             * the device strings from start to end (they are sorted for preference).
             */
            QStringList deviceIds() const;

            /**
             * Returns an icon name used to identify the type of soundcard. Simply use
             * \code
             * KIcon icon(alsaDevice.iconName());
             * \endcode
             * to get the icon.
             */
            QString iconName() const;

            int index() const;

            bool isAvailable() const;

            bool ceaseToExist();

            bool isValid() const;

            bool isCaptureDevice() const;
            bool isPlaybackDevice() const;

        protected:
            AlsaDevice(Solid::AudioHw *audioHw, KSharedConfig::Ptr config);
            AlsaDevice(KConfigGroup &deviceGroup);

        private:
            QSharedDataPointer<AlsaDevicePrivate> d;
    };
} // namespace Phonon
#endif // PHONON_ALSADEVICE_H
