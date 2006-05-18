/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#ifndef SOLID_IFACES_PORTABLEMEDIAPLAYER_H
#define SOLID_IFACES_PORTABLEMEDIAPLAYER_H

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>
#include <solid/ifaces/enums.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This class implement Portable Media Player capability interface and represent
     * a portable media player attached to the system.
     * A portable media player is a portable device able to play multimedia files.
     * Some of them have even recording capabilities.
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class KDE_EXPORT PortableMediaPlayer : virtual public Capability, public Enums::PortableMediaPlayer
    {
//        Q_PROPERTY( AccessType accessMethod READ accessMethod )
//        Q_PROPERTY( QStringList outputFormats READ outputFormats )
//        Q_PROPERTY( QStringList inputFormats READ inputFormats )
//        Q_PROPERTY( QStringList playlistFormats READ playlistFormats )
//        Q_ENUMS( AccessType )

    public:
        /**
         * Destroys a portable media player object.
         */
        virtual ~PortableMediaPlayer();

        /**
         * Retrieves the type of method that should be used to access this
         * device.
         *
         * @return the access method type
         * @see Solid::Ifaces::Enums::PortableMediaPlayer::AccessType
         */
        virtual AccessType accessMethod() const = 0;

        /**
         * Retrieves a list of MIME-types representing the kind of formats
         * that the device can play back.
         *
         * @return a MIME-type list of the supported output formats
         */
        virtual QStringList outputFormats() const = 0;

        /**
         * Retrieves a list of MIME-types representing the kind of formats
         * that the device can record. If empty, it means that the device
         * is not capable of recording.
         *
         * @return a MIME-type list of the supported input formats
         */
        virtual QStringList inputFormats() const = 0;

        /**
         * Retrieves a list of MIME-types representing playlist formats
         * that the device can read.
         *
         * @return a MIME-type list of the supported playlist formats
         */
        virtual QStringList playlistFormats() const = 0;
    };
}
}

#endif
