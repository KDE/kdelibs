
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

#ifndef KDEHW_IFACES_DEVICEMANAGER_H
#define KDEHW_IFACES_DEVICEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <kdehw/ifaces/capability.h>

namespace KDEHW
{
namespace Ifaces
{
    class Device;

    /**
     * This class specifies the interface a backend will have to implement in
     * order to be used in the system.
     *
     * A device manager allow to query the underlying platform to discover the
     * available devices. It has also the responsibility to notify when a device
     * appear or disappear.
     */
    class DeviceManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * Constructs a DeviceManager
         */
        DeviceManager( QObject *parent = 0 );
        /**
         * Destructs a DeviceManager object
         */
        virtual ~DeviceManager();



        /**
         * Retrieves the Universal Device Identifier (UDI) of all the devices
         * available in the system. This identifier is unique for each device
         * in the system.
         *
         * @returns the UDIs of all the devices in the system
         */
        virtual QStringList allDevices() = 0;

        /**
         * Tests if a particular device exists given its UDI.
         *
         * @param udi the identifier of the device tested
         * @returns true if there's a device having this UDI, false otherwise
         */
        virtual bool deviceExists( const QString &udi ) = 0;



        /**
         * Instantiates a new Device object from this backend given its UDI.
         *
         * @param udi the identifier of the device instantiated
         * @returns a new Device object if there's a device having the given UDI, 0 otherwise
         */
        virtual Device *createDevice( const QString &udi ) = 0;



        /**
         * FIXME: Do we really want this here?
         */
        virtual QStringList findDeviceStringMatch( const QString &key, const QString &value ) = 0;

        /**
         * FIXME: Do we really want this here?
         */
        virtual QStringList findDeviceByCapability( const Capability::Type &capability ) = 0;



    signals:
        /**
         * This signal is emitted when a new device appear in the system.
         *
         * @param udi the new device identifier
         */
        void deviceAdded( const QString &udi );

        /**
         * This signal is emitted when a device disappear from the system.
         *
         * @param udi the old device identifier
         */
        void deviceRemoved( const QString &udi );

        /**
         * This signal is emitted when a new capability is detected on a device.
         *
         * @param udi the device identifier
         * @param capability the new capability detected
         */
        void newCapability( const QString &udi, int capability );
    };
}
}

#endif
