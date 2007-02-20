/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#include <kdelibs_export.h>

#include <solid/frontendobject.h>
#include <solid/capability.h>

namespace Solid
{
    class DeviceManager;

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
    class SOLID_EXPORT Device : public FrontendObject
    {
        Q_OBJECT
    public:
        /**
         * This enum type defines the type of change that can occur to a Device
         * property.
         *
         * - PropertyModified : A property value has changed in the device
         * - PropertyAdded : A new property has been added to the device
         * - PropertyRemoved : A property has been removed from the device
         */
        enum PropertyChange { PropertyModified, PropertyAdded, PropertyRemoved };



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
         * Constructs a new device taking its data from a backend.
         *
         * @param backendObject the object given by the backend
         */
        explicit Device( QObject *backendObject );

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
         * Assigns a new value to a given property.
         *
         * Warning: Using this method could expose some backend specific details
         * and lead to non portable code. Use it at your own risk, or during
         * transitional phases when the provided capabilities interfaces don't
         * provide the necessary methods.
         *
         * @param key the key of the property to modify
         * @param value the new value for the property
         * @return true if the change succeeded, false otherwise
         */
        bool setProperty( const QString &key, const QVariant &value );

        /**
         * Retrieves a property of the device.
         *
         * Warning: Using this method could expose some backend specific details
         * and lead to non portable code. Use it at your own risk, or during
         * transitional phases when the provided capabilities interfaces don't
         * provide the necessary methods.
         *
         * @param key the property key
         * @return the actual value of the property, or QVariant() if the
         * property is unknown
         */
        QVariant property( const QString &key ) const;

        /**
         * Retrieves a key/value map of all the known properties for the device.
         *
         * Warning: Using this method could expose some backend specific details
         * and lead to non portable code. Use it at your own risk, or during
         * transitional phases when the provided capabilities interfaces don't
         * provide the necessary methods.
         *
         * @return all the properties of the device
         */
        QMap<QString, QVariant> allProperties() const;

        /**
         * Tests if a property exist in the device.
         *
         * Warning: Using this method could expose some backend specific details
         * and lead to non portable code. Use it at your own risk, or during
         * transitional phases when the provided capabilities interfaces don't
         * provide the necessary methods.
         *
         * @param key the property key
         * @return true if the property is available in the device, false
         * otherwise
         */
        bool propertyExists( const QString &key ) const;

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


        /**
         * Acquires a lock on the device for the given reason.
         *
         * @param reason a message describing the reason for the lock
         * @return true if the lock has been successfully acquired, false otherwise
         */
        bool lock(const QString &reason);

        /**
         * Releases a lock on the device.
         *
         * @return true if the lock has been successfully released
         */
        bool unlock();

        /**
         * Tests if the device is locked.
         *
         * @return true if the device is locked, false otherwise
         */
        bool isLocked() const;

        /**
         * Retrieves the reason for a lock.
         *
         * @return the lock reason if the device is locked, QString() otherwise
         */
        QString lockReason() const;

    Q_SIGNALS:
        /**
         * This signal is emitted when a property is changed in the device.
         *
         * @param changes the map describing the property changes that
         * occurred in the device, keys are property name and values
         * describe the kind of change done on the device property
         * (added/removed/modified), it's one of the type Solid::Device::PropertyChange
         */
        void propertyChanged( const QMap<QString,int> &changes );

        /**
         * This signal is emitted when an event occurred in the device.
         * For example when a button is pressed.
         *
         * @param condition the condition name
         * @param reason a message explaining why the condition has been raised
         */
        void conditionRaised( const QString &condition, const QString &reason );

    protected Q_SLOTS:
        void slotDestroyed( QObject *object );

    private:
        void registerBackendObject( QObject *backendObject );
        void unregisterBackendObject();

        class Private;
        Private * const d;
    };

    typedef QList<Device> DeviceList;
}

#endif
