/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_ACADAPTER_H
#define SOLID_ACADAPTER_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class AcAdapterPrivate;
    class Device;

    /**
     * This device interface is available on AC adapters.
     */
    class SOLID_EXPORT AcAdapter : public DeviceInterface
    {
        Q_OBJECT
        Q_PROPERTY(bool plugged READ isPlugged)
        Q_DECLARE_PRIVATE(AcAdapter)
        friend class Device;

    private:
        /**
         * Creates a new AcAdapter object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit AcAdapter(QObject *backendObject);

    public:
        /**
         * Destroys an AcAdapter object.
         */
        virtual ~AcAdapter();


        /**
         * Get the Solid::DeviceInterface::Type of the AcAdapter device interface.
         *
         * @return the AcAdapter device interface type
         * @see Solid::Ifaces::Enums::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::AcAdapter; }

        /**
         * Indicates if this AC adapter is plugged.
         *
         * @return true if the adapter is plugged, false otherwise
         */
        bool isPlugged() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when the AC adapter is plugged or unplugged.
         *
         * @param newState true if the AC adapter is plugged, false otherwise
         * @param udi the UDI of the AC adapter
         */
        void plugStateChanged(bool newState, const QString &udi);
    };
}

#endif
