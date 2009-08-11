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

#ifndef SOLID_BUTTON_H
#define SOLID_BUTTON_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class ButtonPrivate;
    class Device;

    /**
     * This device interface is available on button devices.
     *
     * A button is a device, like power button or lid switch, that can be pressed by user.
     * Some buttons can have two states (Enabled/Disabled, On/Off ...), other buttons haven't state.
     *
     * @author Davide Bettio <davbet@aliceposta.it>
     */
    class SOLID_EXPORT Button : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(ButtonType)
        Q_PROPERTY(ButtonType type READ type)
        Q_PROPERTY(bool hasState READ hasState)
        Q_PROPERTY(bool stateValue READ stateValue)
        Q_DECLARE_PRIVATE(Button)
        friend class Device;

    public:
        /**
         * This enum type defines the type of button.
         *
         * - LidButton : The switch on a laptop that senses whether the lid is open or closed.
         * - PowerButton : The main power button on the computer.
         * - SleepButton : The sleep button on a computer capable of putting the computer into a suspend state.
	 * - TabletButton : The tablet switch on a laptop that senses if the monitor is rotated or not.
         * - UnknownButtonType : The type of the button is unknow.
         */
         enum ButtonType{ LidButton, PowerButton, SleepButton, UnknownButtonType, TabletButton };


    private:
        /**
         * Creates a new Button object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Button(QObject *backendObject);

    public:
        /**
         * Destroys a Button object.
         */
        virtual ~Button();

        /**
         * Get the Solid::DeviceInterface::Type of the Button device interface.
         *
         * @return the Button device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Button; }

        /**
         * Retrieves the type of button device.
         *
         * @return the type of button device.
         * @see Solid::Button::ButtonType
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
         * Available only if hasState is true.
         *
         * @return true if the button is enabled, false otherwise.
         * @see hasState()
         */
        bool stateValue() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the button is pressed.
         *
         * @param type the type of button device, it's one of
         * the type Solid::Button::ButtonType
         * @see Solid::Button::ButtonType
         * @param udi the UDI of the button
         */
        void pressed(Solid::Button::ButtonType type, const QString &udi);
    };
}

#endif
