/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_PORTABLEMEDIAPLAYER_H
#define SOLID_PORTABLEMEDIAPLAYER_H

#include <QStringList>

#include <solid/solid_export.h>

#include <solid/capability.h>

namespace Solid
{
    class PortableMediaPlayerPrivate;

    /**
     * This class implement Portable Media Player capability interface and represent
     * a portable media player attached to the system.
     * A portable media player is a portable device able to play multimedia files.
     * Some of them have even recording capabilities.
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class SOLID_EXPORT PortableMediaPlayer : public Capability
    {
        Q_OBJECT
        Q_ENUMS( AccessType )
        Q_PROPERTY( AccessType accessMethod READ accessMethod )
        Q_PROPERTY( QStringList outputFormats READ outputFormats )
        Q_PROPERTY( QStringList inputFormats READ inputFormats )
        Q_PROPERTY( QStringList playlistFormats READ playlistFormats )
        Q_DECLARE_PRIVATE(PortableMediaPlayer)

    public:
        /**
         * This enum type defines the access method that can be used for a portable media player
         *
         * - MassStorage : A mass storage portable media player
         * - Proprietary : A portable media player using a proprietary protocol
         */
         enum AccessType { MassStorage, Proprietary };



        /**
         * Creates a new PortableMediaPlayer object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        explicit PortableMediaPlayer( QObject *backendObject );

        /**
         * Destroys a portable media player object.
         */
        virtual ~PortableMediaPlayer();

        /**
         * Get the Solid::Capability::Type of the PortableMediaPlayer capability.
         *
         * @return the PortableMediaPlayer capability type
         * @see Solid::Capability::Type
         */
        static Type capabilityType() { return Capability::PortableMediaPlayer; }

        /**
         * Retrieves the type of method that should be used to access this
         * device.
         *
         * @return the access method type
         * @see Solid::PortableMediaPlayer::AccessType
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

    protected:
        /**
         * @internal
         */
        PortableMediaPlayer(PortableMediaPlayerPrivate &dd, QObject *backendObject);
    };
}

#endif
