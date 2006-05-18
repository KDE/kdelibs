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

#ifndef SOLID_IFACES_ACADAPTER_H
#define SOLID_IFACES_ACADAPTER_H

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on A/C adapters.
     */
    class KDE_EXPORT AcAdapter : virtual public Capability
    {
//         Q_PROPERTY( bool plugged READ isPlugged )

    public:
        /**
         * Destroys an AcAdapter object.
         */
        virtual ~AcAdapter();


        /**
         * Indicates if this A/C adapter is plugged.
         *
         * @return true if the adapter is plugged, false otherwise
         */
        virtual bool isPlugged() const = 0;

    protected:
    //signals:
        /**
         * This signal is emitted when the A/C adapter is plugged or unplugged.
         *
         * @param newState true if the A/C adapter is plugged is mounted, false otherwise
         */
        virtual void plugStateChanged( bool newState ) = 0;
    };
}
}

#endif
