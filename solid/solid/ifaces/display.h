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

#ifndef SOLID_IFACES_DISPLAY_H
#define SOLID_IFACES_DISPLAY_H

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This capability is available on display devices.
     *
     * A display is a device, like CRT monitor, LCD screen and TVs, able to show something to the user.
     *
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class KDE_EXPORT Display : virtual public Capability, public Enums::Display
    {
//         Q_PROPERTY( DisplayType type READ type )
//         Q_PROPERTY( int lcdBrightness READ lcdBrightness )
//         Q_ENUMS( DisplayType )

    public:
        /**
         * Destroys a Display object.
         */
        virtual ~Display();

        /**
         * Retrieves the type of display device.
         *
         * @return the type of display device.
         * @see Solid::Ifaces::Enums::Display::DisplayType
         */
        virtual DisplayType type() const = 0;

        /**
         * Retrieves the brightness level in percent.
         * Avaible only if displayType is lcd.
         *
         * @return the brightness level in percent. If displayType is different from lcd, return 100.
         */
        virtual int lcdBrightness() const = 0;
    };
}
}

Q_DECLARE_INTERFACE( Solid::Ifaces::Display, "org.kde.Solid.Ifaces.Display/0.1" )

#endif
