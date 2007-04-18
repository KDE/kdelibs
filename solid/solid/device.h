/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_DEVICE_H
#define SOLID_DEVICE_H

#include <QVariant>
#include <QString>
#include <QMap>
#include <QList>
#include <QtCore/QObject>

#include <solid/solid_export.h>

#include <solid/capability.h>

namespace Solid
{
    class DeviceManager;
    class DevicePrivate;

    /**
     * This class allows applications to deal with devices available in the
     * underlying system.
     *
     * Device stores a reference to device data provided by the backend.
     * Pointers on Device objects are generally not needed, copying such objects
     * is quite cheap.
     *
     *
     * Warning: This class provides methods related to device properties,
     * using these methods could expose some backend specific details
     * and lead to non portable code. Use them at your own risk, or during
     * transitional phases when the provided capabilities interfaces don't
     * provide the necessary methods.
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    class SOLID_EXPORT Device : public QObject
    {
        Q_OBJECT
    public:

        /**
         * Constructs an invalid device.
         */
        Device();

        /**
         * Constructs a device for a given Universal Device Identifier (UDI).
         *
         * @param udi the udi of the device to create
         */
        explicit Device( const QString &udi );

        /**
         * Constructs a copy of a device.
         *
         * @param device the device to copy
         */
        Device( const Device &device );

        /**
         * Destroys the device.
         */
        ~Device();



        /**
         * Assigns a device to this device and returns a reference to it.
         *
         * @param device the device to assign
         * @return a reference to the device
         */
        Device &operator=( const Device &device );

        /**
         * Indicates if this device is valid.
         * A device is considered valid if it's available in the system.
         *
         * @return true if this device is available, false otherwise
         */
        bool isValid() const;


        /**
         * Retrieves the Universal Device Identifier (UDI).
         *
         * @return the udi of the device
         */
        QString udi() const;

        /**
         * Retrieves the Universal Device Identifier (UDI)
         * of the Device's parent.
         *
         * @return the udi of the device's parent
         */
        QString parentUdi() const;


        /**
         * Retrieves the parent of the Device.
         *
         * @return the device's parent
         * @see parentUdi()
         * @see DeviceManager::findDevice()
         */
        Device parent() const;



        /**
         * Retrieves the name of the device vendor.
         *
         * @return the vendor name
         */
        QString vendor() const;

        /**
         * Retrieves the name of the product corresponding to this device.
         *
         * @return the product name
         */
        QString product() const;



        /**
         * Tests if a capability is available from the device.
         *
         * @param capability the capability to query
         * @return true if the capability is available, false otherwise
         */
        bool queryCapability( const Capability::Type &capability ) const;

        /**
         * Retrieves a specialized interface to interact with the device corresponding to
         * a particular capability.
         *
         * @param capability the capability type
         * @returns a pointer to the capability interface if it exists, 0 otherwise
         */
        Capability *asCapability( const Capability::Type &capability );

        /**
         * Retrieves a specialized interface to interact with the device corresponding to
         * a particular capability.
         *
         * @param capability the capability type
         * @returns a pointer to the capability interface if it exists, 0 otherwise
         */
        const Capability *asCapability( const Capability::Type &capability ) const;

        /**
         * Retrieves a specialized interface to interact with the device corresponding
         * to a given capability interface.
         *
         * @returns a pointer to the capability interface if it exists, 0 otherwise
         */
        template <class Cap> Cap *as()
        {
            Capability::Type type = Cap::capabilityType();
            Capability *iface = asCapability( type );
            return qobject_cast<Cap*>( iface );
        }

        /**
         * Retrieves a specialized interface to interact with the device corresponding
         * to a given capability interface.
         *
         * @returns a pointer to the capability interface if it exists, 0 otherwise
         */
        template <class Cap> const Cap *as() const
        {
            Capability::Type type = Cap::capabilityType();
            const Capability *iface = asCapability( type );
            return qobject_cast<const Cap*>( iface );
        }

        /**
         * Tests if a device provides a given capability interface.
         *
         * @returns true if the interface is available, false otherwise
         */
        template <class Cap> bool is()
        {
            return queryCapability( Cap::capabilityType() );
        }

    private:
        Q_PRIVATE_SLOT(d, void _k_destroyed(QObject*))

        DevicePrivate *d;
        friend class DeviceManagerPrivate;
    };

    typedef QList<Device> DeviceList;
}

#endif
