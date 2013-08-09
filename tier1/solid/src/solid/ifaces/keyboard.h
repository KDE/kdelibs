/*
    Copyright 2013 Ivan Cukic <ivan.cukic@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_IFACES_KEYBOARD_H
#define SOLID_IFACES_KEYBOARD_H

#include <solid/ifaces/deviceinterface.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on keyboards.
     *
     * A keyboard is an input device.
     *
     * @author Ivan Cukic <ivan.cukic@kde.org>
     */
    class Keyboard : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a Keyboard object.
         */
        virtual ~Keyboard();

        /**
         * Retrieves the model of the keyboard (for example 'pc105')
         * @return the keyboard model
         */
        virtual QString model() const = 0;

        /**
         * Retrieves the layout of the keyboard (for example 'us')
         * @return the layout of the keyboard
         */
        virtual QString layout() const = 0;

    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::Keyboard, "org.kde.Solid.Ifaces.Keyboard/0.1")

#endif
