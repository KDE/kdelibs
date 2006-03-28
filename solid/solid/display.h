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

#ifndef KDEHW_DISPLAY_H
#define KDEHW_DISPLAY_H

#include <kdehw/capability.h>

namespace KDEHW
{
    namespace Ifaces
    {
        class Display;
    }

    /**
     * This class implement Display capability interface and represent display
     * devices attached to the system.
     * A display is a device, like CRT monitor, LCD screen and TVs, able to show something to the user.
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class Display : public Capability, public Ifaces::Enums::Display
    {
        Q_OBJECT
    public:
        /**
         * Constructs a Display.
         */
        Display( Ifaces::Display *iface, QObject *parent = 0 );

        /**
         * Destruct the Display object.
         */
        virtual ~Display();

        /**
         * Get the capability type (Display).
         * @returns Capability::Display.
         */
        static Type type() { return Capability::Display; }

        /**
         * Get the type of display device.
         * @returns the type of display device.
         */
        DisplayType displayType() const;

        /**
         * Get the brightness level in percent.
         * Avaible only if displayType is lcd.
         * @returns the brightness level in percent. If displayType is different from lcd, return 100.
         */
        int lcdBrightness() const;

    private:
        class Private;
        Private *d;
    };
}

#endif
