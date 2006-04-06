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

#ifndef KDEHW_PORTABLEMEDIAPLAYER_H
#define KDEHW_PORTABLEMEDIAPLAYER_H

#include <QStringList>

#include <kdehw/capability.h>
#include <kdehw/ifaces/enums.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class PortableMediaPlayer;
    }

    /**
     * This class implement Portable Media Player capability interface and represent
     * a portable media player attached to the system.
     * A portable media player is a portable device able to play multimedia files.
     * Some of them have even recording capabilities.
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class PortableMediaPlayer : public Capability, public Ifaces::Enums::PortableMediaPlayer
    {
        Q_OBJECT
    public:
        /**
         * Creates a new PortableMediaPlayer object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param iface the capability interface provided by the backend
         * @param parent the parent QObject
         * @see KDEHW::Device::as()
         */
        PortableMediaPlayer( Ifaces::PortableMediaPlayer *iface, QObject *parent = 0 );

        /**
         * Destroys a portable media player object.
         */
        virtual ~PortableMediaPlayer();

        /**
         * Get the KDEHW::Capability::Type of the PortableMediaPlayer capability.
         *
         * @return the PortableMediaPlayer capability type
         * @see KDEHW::Ifaces::Enums::Capability::Type
         */
        static Type type() { return Capability::PortableMediaPlayer; };

        /**
         * Retrieves the type of method that should be used to access this
         * device.
         *
         * @return the access method type
         * @see KDEHW::Ifaces::Enums::PortableMediaPlayer::AccessType
         */
        AccessType accessMethod() const;

        /**
         * Retrieves a list of MIME-types representing the kind of formats
         * that the device can play back.
         *
         * @return a MIME-type list of the supported output formats
         */
        QStringList outputFormats() const;

        /**
         * Retrieves a list of MIME-types representing the kind of formats
         * that the device can record. If empty, it means that the device
         * is not capable of recording.
         *
         * @return a MIME-type list of the supported input formats
         */
        QStringList inputFormats() const;

        /**
         * Retrieves a list of MIME-types representing playlist formats
         * that the device can read.
         *
         * @return a MIME-type list of the supported playlist formats
         */
        QStringList playlistFormats() const;

    private:
        class Private;
        Private *d;
    };
};

#endif
