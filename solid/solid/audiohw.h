/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_AUDIOHW_H
#define SOLID_AUDIOHW_H

#include <kdelibs_export.h>

#include <solid/capability.h>

namespace Solid
{
    class AudioHwPrivate;

    /**
     * This capability is available on interfaces exposed by sound cards.
     */
    class SOLID_EXPORT AudioHw : public Capability
    {
        Q_OBJECT
        Q_ENUMS( AudioDriver AudioHwType SoundcardType )
        Q_FLAGS( AudioHwTypes )
        Q_PROPERTY( AudioDriver driver READ driver )
        Q_PROPERTY( QStringList driverHandles READ driverHandles )
        Q_PROPERTY( QString name READ name )
        Q_PROPERTY( AudioHwTypes deviceType READ deviceType )
        Q_PROPERTY( SoundcardType soundcardType READ soundcardType )

    public:
        /**
         * This enum type defines the type of driver required to
         * interact with the device.
         */
        enum AudioDriver
        {
            /**
             * An Advanced Linux Sound Architecture (ALSA) driver device
             */
            Alsa,
            /**
             * An Open Sound System (OSS) driver device
             */
            OpenSoundSystem,
            /**
             * An unknown driver device
             */
            UnknownAudioDriver
        };

        /**
         * This enum type defines the type of audio interface this
         * device expose.
         */
        enum AudioHwType
        {
            /**
             * An unknown audio interface
             */
            UnknownAudioHwType = 0,
            /**
             * A control/mixer interface
             */
            AudioControl = 1,
            /**
             * An audio source
             */
            AudioInput = 2,
            /**
             * An audio sink
             */
            AudioOutput = 4
        };

        /**
         * This type stores an OR combination of AudioHwType values.
         */
        Q_DECLARE_FLAGS( AudioHwTypes, AudioHwType )

        /**
         * This enum defines the type of soundcard of this device.
         */
        enum SoundcardType {
            /**
             * An internal soundcard (onboard or PCI card).
             */
            InternalSoundcard,
            /**
             * An external USB soundcard (that is not a headphone).
             */
            UsbSoundcard,
            /**
             * An external Firewire soundcard.
             */
            FirewireSoundcard,
            /**
             * A headset attached to a USB port or connected via Bluetooth (the headset includes its
             * own audio hardware; it is impossible to detect a headset connected to the internal
             * soundcard).
             */
            Headset,
        };



        /**
         * Creates a new AudioHw object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        explicit AudioHw( QObject *backendObject );

        /**
         * Destroys an AudioHw object.
         */
        virtual ~AudioHw();


        /**
         * Get the Solid::Capability::Type of the AudioHw capability.
         *
         * @return the AudioHw capability type
         * @see Solid::Capability::Type
         */
        static Type capabilityType() { return Capability::AudioHw; }



        /**
         * Retrieves the audio driver that should be used to access the device.
         *
         * @return the driver needed to access the device
         * @see Solid::AudioHw::AudioDriver
         */
        AudioDriver driver();

        /**
         * Retrieves a driver specific string allowing to access the device.
         *
         * For example for Alsa devices it is of the form "hw:0,0"
         * while for OSS it is "/dev/foo".
         *
         * @return the driver specific string to handle this device
         */
        QStringList driverHandles();



        /**
         * Retrieves the name of this audio interface.
         *
         * @return the name of the audio interface if available, QString() otherwise
         */
        QString name();

        /**
         * Retrieves the type of this audio interface (in/out/control).
         *
         * @return the type of this audio interface
         * @see Solid::AudioHw::AudioHwTypes
         */
        AudioHwTypes deviceType();

        /**
         * Retrieves the type of soundcard (internal/headset/...).
         *
         * @return the type of soundcard
         * @see Solid::AudioHw::SoundcardType
         */
        SoundcardType soundcardType();

    private:
        AudioHwPrivate *d;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS( Solid::AudioHw::AudioHwTypes )

#endif
