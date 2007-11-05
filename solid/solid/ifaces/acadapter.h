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

#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on AC adapters.
     */
    class AcAdapter : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys an AcAdapter object.
         */
        virtual ~AcAdapter();


        /**
         * Indicates if this AC adapter is plugged.
         *
         * @return true if the adapter is plugged, false otherwise
         */
        virtual bool isPlugged() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the AC adapter is plugged or unplugged.
         *
         * @param newState true if the AC adapter is plugged is mounted, false otherwise
         * @param udi the UDI of the AC adapter with the changed plugging state
         */
        virtual void plugStateChanged(bool newState, const QString &udi) = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::AcAdapter, "org.kde.Solid.Ifaces.AcAdapter/0.1")

#endif
