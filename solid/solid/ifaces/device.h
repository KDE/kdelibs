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

#ifndef SOLID_IFACES_DEVICE_H
#define SOLID_IFACES_DEVICE_H

#include <QObject>
#include <QVariant>
#include <QString>
#include <QMap>

#include <solid/solid_export.h>

#include <solid/capability.h>
#include <solid/device.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This class specifies the interface a device will have to comply to in order to be used in the system.
     *
     * Backends will have to implement it to gather and modify data in the underlying system.
     * Each device has a set of key/values pair describing its properties. It has also a list of capabilities
     * describing what the device actually is (a cdrom drive, a portable media player, etc.)
     *
     * @author Kevin Ottens <ervin@kde.org>
     */
    class SOLIDIFACES_EXPORT Device : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructs a Device
         */
        Device( QObject *parent = 0 );
        /**
         * Destruct the Device object
         */
        virtual ~Device();


        /**
         * Retrieves the Universal Device Identifier (UDI) of the Device.
         * This identifier is unique for each device in the system.
         *
         * @returns the Universal Device Identifier of the current device
         */
        virtual QString udi() const = 0;

        /**
         * Retrieves the Universal Device Identifier (UDI) of the Device's
         * parent.
         *
         * @returns the Universal Device Identifier of the parent device
         */
        virtual QString parentUdi() const;


        /**
         * Retrieves the name of the device vendor.
         *
         * @return the vendor name
         */
        virtual QString vendor() const = 0;

        /**
         * Retrieves the name of the product corresponding to this device.
         *
         * @return the product name
         */
        virtual QString product() const = 0;


        /**
         * Changes the value of a property.
         *
         * @param key the property name
         * @param value the new value of this property
         * @returns true if the operation succeeded, false otherwise
         */
        virtual bool setProperty( const QString &key, const QVariant &value );

        /**
         * Retrieves the value of a property.
         *
         * @param key the property name
         * @returns the property value or QVariant() if the property doesn't exist
         */
        virtual QVariant property( const QString &key ) const = 0;

        /**
         * Retrieves all the properties of this device.
         *
         * @returns all properties in a map
         */
        virtual QMap<QString, QVariant> allProperties() const = 0;

        /**
         * Removes a property.
         *
         * @param key the property name
         * @returns true if the operation succeeded, false otherwise
         */
        virtual bool removeProperty( const QString &key );

        /**
         * Tests if a property exist.
         *
         * @param key the property name
         * @returns true if the property exists in this device, false otherwise
         */
        virtual bool propertyExists( const QString &key ) const = 0;

        /**
         * Adds a capability to this device.
         *
         * @param capability the capability type
         * @returns true if the operation succeeded, false otherwise
         */
        virtual bool addCapability( const Solid::Capability::Type &capability );

        /**
         * Tests if a property exist.
         *
         * @param capability the capability type
         * @returns true if the capability is provided by this device, false otherwise
         */
        virtual bool queryCapability( const Solid::Capability::Type &capability ) const = 0;

        /**
         * Create a specialized interface to interact with the device corresponding to
         * a particular capability.
         *
         * @param capability the capability type
         * @returns a pointer to the capability interfaces if supported by the device, 0 otherwise
         */
        virtual QObject *createCapability( const Solid::Capability::Type &capability ) = 0;

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
    };
}
}

#endif
