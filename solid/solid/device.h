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

#ifndef KDEHW_DEVICE_H
#define KDEHW_DEVICE_H

#include <QVariant>
#include <QString>
#include <QMap>
#include <QList>

#include <kdehw/ifaces/device.h>
#include <kdehw/ifaces/capability.h>

namespace KDEHW
{
    using Ifaces::Capability;
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
    class Device : public Ifaces::Device
    {
        Q_OBJECT
    public:
        /**
         * Constructs an invalid device.
         */
        Device();

        /**
         * Constructs a copy of a device.
         *
         * @param device the device to copy
         */
        Device( const Device &device );

        /**
         * Constructs a new device taking its data from a backend.
         *
         * @param data the data given by the backend
         */
        Device( Ifaces::Device *data );

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
         * Indicates if a device is valid. A device is considered valid if it
         * refers to a udi corresponding to a known device in the underlying
         * system.
         *
         * @return true if the device is valid, false otherwise
         */
        bool isValid() const;


        /**
         * Retrieves the Universal Device Identifier (UDI).
         *
         * @return the udi of the device
         */
        virtual QString udi() const;

        /**
         * Retrieves the Universal Device Identifier (UDI)
         * of the Device's parent.
         *
         * @return the udi of the device's parent
         */
        virtual QString parentUdi() const;


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
        virtual QString vendor() const;

        /**
         * Retrieves the name of the product corresponding to this device.
         *
         * @return the product name
         */
        virtual QString product() const;




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
        virtual bool setProperty( const QString &key, const QVariant &value );

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
        virtual QVariant property( const QString &key ) const;

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
        virtual QMap<QString, QVariant> allProperties() const;

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
        virtual bool propertyExists( const QString &key ) const;

        /**
         * Tests if a capability is available from the device.
         *
         * @param capability the capability to query
         * @return true if the capability is available, false otherwise
         */
        virtual bool queryCapability( const Capability::Type &capability ) const;

        /**
         * Retrieves a specialized interface to interact with the device corresponding to
         * a particular capability.
         *
         * @param capability the capability type
         * @returns a pointer to the capability interface if it exists, 0 otherwise
         */
        virtual Ifaces::Capability *asCapability( const Capability::Type &capability );

        /**
         * Retrieves a specialized interface to interact with the device corresponding
         * to a given capability interface.
         *
         * @returns a pointer to the capability interface if it exists, 0 otherwise
         */
        template <class Cap> Cap *as()
        {
            Capability::Type type = Cap::type();
            Ifaces::Capability *iface = asCapability( type );
            return dynamic_cast<Cap*>( iface );
        }

        /**
         * Tests if a device provides a given capability interface.
         *
         * @returns true if the interface is available, false otherwise
         */
        template <class Cap> bool is()
        {
            return queryCapability( Cap::type() );
        }


        /**
         * Acquires a lock on the device for the given reason.
         *
         * @param reason a message describing the reason for the lock
         * @return true if the lock has been successfully acquired, false otherwise
         */
        virtual bool lock(const QString &reason);

        /**
         * Releases a lock on the device.
         *
         * @return true if the lock has been successfully released
         */
        virtual bool unlock();

        /**
         * Tests if the device is locked.
         *
         * @return true if the device is locked, false otherwise
         */
        virtual bool isLocked() const;

        /**
         * Retrieves the reason for a lock.
         *
         * @return the lock reason if the device is locked, QString() otherwise
         */
        virtual QString lockReason() const;

    private slots:
        void slotPropertyChanged( const QMap<QString,int> &changes );
        void slotConditionRaised( const QString &condition, const QString &reason );
        void slotDestroyed( QObject *object );

    private:
        class Private;
        Private *d;
    };

    typedef QList<Device> DeviceList;
}

#endif
