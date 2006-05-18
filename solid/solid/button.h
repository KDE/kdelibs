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

#ifndef SOLID_BUTTON_H
#define SOLID_BUTTON_H

#include <kdelibs_export.h>

#include <solid/capability.h>

namespace Solid
{
    namespace Ifaces
    {
        class Button;
    }

    /**
     * This capability is available on button devices.
     *
     * A button is a device, like power button or lid switch, that can be pressed by user.
     * Some buttons can have two states (Enabled/Disabled, On/Off ...), other buttons haven't state.
     *
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class KDE_EXPORT Button : public Capability, public Ifaces::Enums::Button
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Button object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param iface the capability interface provided by the backend
         * @param parent the parent QObject
         * @see Solid::Device::as()
         */
        Button( Ifaces::Button *iface, QObject *parent = 0 );

        /**
         * Destroys a Button object.
         */
        virtual ~Button();

        /**
         * Get the Solid::Capability::Type of the Button capability.
         *
         * @return the Button capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::Button; }

        /**
         * Retrieves the type of button device.
         *
         * @return the type of button device.
         * @see Solid::Ifaces::Enums::Button::ButtonType
         */
        ButtonType type() const;

        /**
         * Indicates if the button mantains state (Can toggled on/off).
         *
         * @return true if the button maintains state, false otherwise.
         * @see stateValue()
         */
        bool hasState() const;

        /**
         * Retrieves the state of the button.
         * A button can have two states (Enabled/Disabled, On/Off ...).
         * Avaible only if hasState is true.
         *
         * @return true if the button is enabled, false otherwise.
         * @see hasState()
         */
        bool stateValue() const;

    signals:
        /**
         * This signal is emitted when the button is pressed.
         *
         * @param type the type of button device, it's one of
         * the type Solid::Ifaces::Enums::Button::ButtonType
         * @see Solid::Ifaces::Enums::Button::ButtonType
         */
        void pressed( int type );

    private slots:
        void slotPressed( int type );

    private:
        class Private;
        Private *d;
    };
}

#endif
