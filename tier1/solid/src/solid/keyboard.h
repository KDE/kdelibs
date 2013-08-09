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

#ifndef SOLID_KEYBOARD_H
#define SOLID_KEYBOARD_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class KeyboardPrivate;
    class Device;

    /**
     * This device interface is available on keyboards.
     *
     * A keyboard is an input device.
     *
     * @author Ivan Cukic <ivan.cukic@kde.org>
     */
    class SOLID_EXPORT Keyboard : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(QString model READ model)
        Q_PROPERTY(QString layout READ layout)
        Q_DECLARE_PRIVATE(Keyboard)
        friend class Device;

    private:
        /**
         * Creates a new Keyboard object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit Keyboard(QObject *backendObject);

    public:
        /**
         * Destroys a Keyboard object.
         */
        virtual ~Keyboard();

        /**
         * Get the Solid::DeviceInterface::Type of the Keyboard device interface.
         *
         * @return the Keyboard device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::Keyboard; }

        /**
         * Retrieves the layout of the keyboard (for example 'us')
         * @return the layout of the keyboard
         */
        QString layout() const;

        /**
         * Retrieves the model of the keyboard (for example 'pc105')
         * @return the keyboard model
         */
        QString model() const;

    };
}

#endif
