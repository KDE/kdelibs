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

#ifndef SOLID_DISPLAY_H
#define SOLID_DISPLAY_H

#include <kdelibs_export.h>

#include <solid/capability.h>

namespace Solid
{
    /**
     * This capability is available on display devices.
     *
     * A display is a device, like CRT monitor, LCD screen and TVs, able to show something to the user.
     *
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class SOLID_EXPORT Display : public Capability
    {
        Q_OBJECT
        Q_ENUMS( DisplayType )
        Q_PROPERTY( DisplayType type READ type )
        Q_PROPERTY( int lcdBrightness READ lcdBrightness )

    public:
        /**
         * This enum type defines the type of display a device can be.
         *
         * - Lcd: A LCD (Liquid Crystal Display) panel
         * - Crt: A CRT (Cathode Ray Tube) tube
         * - TvOut: A TV Out port
         * - UnknowDisplayType: An undetermined display type
         */
        enum DisplayType{ Lcd, Crt, TvOut, UnknownDisplayType };



        /**
         * Creates a new Display object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Display( QObject *backendObject );

        /**
         * Destroys a Display object.
         */
        virtual ~Display();

        /**
         * Get the Solid::Capability::Type of the Button capability.
         *
         * @return the Button capability type
         * @see Solid::Capability::Type
         */
        static Type capabilityType() { return Capability::Display; }

        /**
         * Retrieves the type of display device.
         *
         * @return the type of display device.
         * @see Solid::Display::DisplayType
         */
        DisplayType type() const;

        /**
         * Retrieves the brightness level in percent.
         * Available only if displayType is lcd.
         *
         * @return the brightness level in percent. If displayType is different from lcd, return 100.
         */
        int lcdBrightness() const;

    private:
        class Private;
        Private * const d;
    };
}

#endif
