/*  This file is part of the KDE project
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

#ifndef SOLID_IFACES_DVBHW_H
#define SOLID_IFACES_DVBHW_H

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on Digital Video Broadcast (DVB) devices.
     *
     * A DVB device is a device implementing the open standards for digital
     * television maintained by the DVB Project
     * It is possible to interact with such a device using a special device
     * file in the system.
     */
    class SOLIDIFACES_EXPORT DvbHw : virtual public Capability
    {
    public:
        /**
         * Destroys a DvbHw object.
         */
        virtual ~DvbHw();

        /**
         * Retrieves the absolute path of the special file to interact
         * with the device.
         *
         * @return the absolute path of the special file to interact with
         * the device
         */
        virtual QString device() const = 0;
    };
}
}

Q_DECLARE_INTERFACE( Solid::Ifaces::DvbHw, "org.kde.Solid.Ifaces.DvbHw/0.1" )

#endif
