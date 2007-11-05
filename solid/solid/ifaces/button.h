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

#ifndef SOLID_IFACES_BUTTON_H
#define SOLID_IFACES_BUTTON_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/button.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on button devices.
     *
     * A button is a device, like power button or lid switch, that can be pressed by user.
     * Some buttons can have two states (Enabled/Disabled, On/Off ...), other buttons haven't state.
     *
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class Button : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a Button object.
         */
        virtual ~Button();

        /**
         * Retrieves the type of button device.
         *
         * @return the type of button device.
         * @see Solid::Button::ButtonType
         */
        virtual Solid::Button::ButtonType type() const = 0;

        /**
         * Indicates if the button mantains state (Can toggled on/off).
         *
         * @return true if the button maintains state, false otherwise.
         * @see stateValue()
         */
        virtual bool hasState() const = 0;

        /**
         * Retrieves the state of the button.
         * A button can have two states (Enabled/Disabled, On/Off ...).
         * Available only if hasState is true.
         *
         * @return true if the button is enabled, false otherwise.
         * @see hasState()
         */
        virtual bool stateValue() const = 0;

    protected:
    //Q_SIGNALS:
        /**
         * This signal is emitted when the button is pressed.
         *
         * @param type the type of button device, it's one of
         * the type Solid::Button::ButtonType
         * @see Solid::Button::ButtonType
         */
        virtual void pressed(Solid::Button::ButtonType type, const QString &udi) = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Button, "org.kde.Solid.Ifaces.Button/0.1")

#endif
