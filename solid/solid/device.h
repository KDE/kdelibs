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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KDEHW_DEVICE_H
#define KDEHW_DEVICE_H

#include <QVariant>
#include <QString>
#include <QMap>
#include <QList>

#include "ifaces/device.h"

namespace KDEHW
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
    class Device : public QObject
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
        QString udi() const;


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
         * FIXME: At one point we'll want to switch to an enum or something similar,
         * QString is not here to stay.
         *
         * @param capability the capability to query
         * @return true if the capability is available, false otherwise
         */
        bool queryCapability( const QString &capability ) const;



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

    signals:
        /**
         * This signal is emitted when a property changed in the device.
         *
         * @param key the property key
         * @param change the type of change that happened to the property,
         * it's one of the type KDEHW::PropertyChange
         */
        void propertyChanged( const QString &key, int change );

        /**
         * This signal is emitted when a condition has been met in the device.
         *
         * @param condition the condition name
         * @param reason the reason for the condition
         */
        void conditionRaised( const QString &condition, const QString &reason );

    private slots:
        void slotPropertyChanged( const QString &key, int change );
        void slotConditionRaised( const QString &condition, const QString &reason );
        void slotDestroyed( QObject *object );

    private:
        class Private;
        Private *d;
    };

    typedef QList<Device> DeviceList;
}

#endif
