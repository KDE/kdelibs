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

#ifndef SOLID_IFACES_AUDIOHW_H
#define SOLID_IFACES_AUDIOHW_H

#include <solid/solid_export.h>

#include <solid/ifaces/capability.h>
#include <solid/audiohw.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on interfaces exposed by sound cards.
     */
    class SOLIDIFACES_EXPORT AudioHw : virtual public Capability
    {
    public:
        /**
         * Destroys an AudioHw object.
         */
        virtual ~AudioHw();



        /**
         * Retrieves the audio driver that should be used to access the device.
         *
         * @return the driver needed to access the device
         * @see Solid::AudioHw::AudioDriver
         */
        virtual Solid::AudioHw::AudioDriver driver() const = 0;

        /**
         * Retrieves a driver specific string allowing to access the device.
         *
         * For example for Alsa devices it is of the form "hw:0,0"
         * while for OSS it is "/dev/foo".
         *
         * @return the driver specific string to handle this device
         */
        virtual QString driverHandler() const = 0;



        /**
         * Retrieves the name of this audio interface.
         *
         * @return the name of the audio interface if available, QString() otherwise
         */
        virtual QString name() const = 0;

        /**
         * Retrieves the type of this audio interface.
         *
         * @return the type of this audio interface
         * @see Solid::AudioHw::AudioHwTypes
         */
        virtual Solid::AudioHw::AudioHwTypes deviceType() const = 0;

        /**
         * Retrieves the type of soundcard (internal/headset/...).
         *
         * @return the type of soundcard
         * @see Solid::AudioHw::SoundcardType
         */
        virtual Solid::AudioHw::SoundcardType soundcardType() const = 0;
    };
}
}

Q_DECLARE_INTERFACE( Solid::Ifaces::AudioHw, "org.kde.Solid.Ifaces.AudioHw/0.1.1" )

#endif
