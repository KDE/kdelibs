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

#ifndef KDEHW_IFACES_PORTABLEMEDIAPLAYER_H
#define KDEHW_IFACES_PORTABLEMEDIAPLAYER_H

#include <kdehw/ifaces/capability.h>
#include <kdehw/ifaces/enums.h>

namespace KDEHW
{
namespace Ifaces
{
    class PortableMediaPlayer : virtual public Capability, public Enums::PortableMediaPlayer
    {
//        Q_PROPERTY( AccessType accessMethod READ accessMethod )
//        Q_PROPERTY( QStringList outputFormats READ outputFormats )
//        Q_PROPERTY( QStringList inputFormats READ inputFormats )
//        Q_PROPERTY( QStringList playlistFormats READ playlistFormats )
//        Q_ENUMS( AccessType )

    public:
        virtual ~PortableMediaPlayer();

        virtual AccessType accessMethod() const = 0;
        virtual QStringList outputFormats() const = 0;
        virtual QStringList inputFormats() const = 0;
        virtual QStringList playlistFormats() const = 0;
    };
}
}

#endif
