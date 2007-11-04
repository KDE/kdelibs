/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_AUDIOINTERFACE_H
#define SOLID_AUDIOINTERFACE_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

class QVariant;
namespace Solid
{
    class AudioInterfacePrivate;
    class Device;

    /**
     * This device interface is available on interfaces exposed by sound cards.
     */
    class SOLID_EXPORT AudioInterface : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(AudioDriver AudioInterfaceType SoundcardType)
        Q_FLAGS(AudioInterfaceTypes)
        Q_PROPERTY(AudioDriver driver READ driver)
        Q_PROPERTY(QVariant driverHandle READ driverHandle)
        Q_PROPERTY(QString name READ name)
        Q_PROPERTY(AudioInterfaceTypes deviceType READ deviceType)
        Q_PROPERTY(SoundcardType soundcardType READ soundcardType)
        Q_DECLARE_PRIVATE(AudioInterface)
        friend class Device;

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
        enum AudioInterfaceType
        {
            /**
             * An unknown audio interface
             */
            UnknownAudioInterfaceType = 0,
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
         * This type stores an OR combination of AudioInterfaceType values.
         */
        Q_DECLARE_FLAGS(AudioInterfaceTypes, AudioInterfaceType)

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
            /**
             * A modem device.
             *
             * Softmodems today are implemented as (cheap) soundcards. ALSA can provide an interface
             * to the modem.
             */
            Modem
        };


    private:
        /**
         * Creates a new AudioInterface object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit AudioInterface(QObject *backendObject);

    public:
        /**
         * Destroys an AudioInterface object.
         */
        virtual ~AudioInterface();


        /**
         * Get the Solid::DeviceInterface::Type of the AudioInterface device interface.
         *
         * @return the AudioInterface device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::AudioInterface; }



        /**
         * Retrieves the audio driver that should be used to access the device.
         *
         * @return the driver needed to access the device
         * @see Solid::AudioInterface::AudioDriver
         */
        AudioDriver driver() const;

        /**
         * Retrieves a driver specific handle to access the device.
         *
         * For Alsa devices it is a list with (card, device, subdevice).
         * \code
         * QVariantList list = dev->driverHandle().toList();
         * QString card = list[0].toString();
         * int device = list[1].toInt();
         * int subdevice = list[2].toInt();
         * \endcode
         * The card entry sometimes can be converted to an integer, but it may just as well be the
         * textual id for the card. So don't rely on it to work with QVariant::toInt().
         *
         * For OSS devices it is simply a string like "/dev/dsp". Use QVariant::toString() to
         * retrieve the string.
         *
         * @return the driver specific data to handle this device
         */
        QVariant driverHandle() const;



        /**
         * Retrieves the name of this audio interface.
         *
         * The product name of the parent device is normally better suited for the user to identify
         * the soundcard. If the soundcard has multiple devices, though you need to add this name to
         * differentiate between the devices.
         *
         * @return the name of the audio interface if available, QString() otherwise
         */
        QString name() const;

        /**
         * Retrieves the type of this audio interface (in/out/control).
         *
         * @return the type of this audio interface
         * @see Solid::AudioInterface::AudioInterfaceTypes
         */
        AudioInterfaceTypes deviceType() const;

        /**
         * Retrieves the type of soundcard (internal/headset/...).
         *
         * @return the type of soundcard
         * @see Solid::AudioInterface::SoundcardType
         */
        SoundcardType soundcardType() const;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::AudioInterface::AudioInterfaceTypes)

#endif // SOLID_AUDIOINTERFACE_H
