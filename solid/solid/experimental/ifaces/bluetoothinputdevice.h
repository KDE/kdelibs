/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2007 Daniel Gollub <dgollub@suse.de>


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

#ifndef SOLID_IFACES_BLUETOOTHINPUTDEVICE
#define SOLID_IFACES_BLUETOOTHINPUTDEVICE

#include <QObject>
#include <solid/solid_export.h>

namespace SolidExperimental
{
namespace Ifaces
{
/**
 * A BluetoothInputDevice object allows to manage the connection of a bluetooth input device.
 */
class SOLIDIFACES_EXPORT BluetoothInputDevice : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a BluetoothInputDevice.
     *
     * @param parent the parent object
     */
    BluetoothInputDevice(QObject * parent = 0);

    /**
     * Destructs a BluetoothInputDevice object.
     */
    virtual ~BluetoothInputDevice();

    /**
     * Retrieves ubi of bluetooth input device.
     *
     * @returns ubi of bluetooth input device
     */
    virtual QString ubi() const = 0;

    /**
     * Retrieves connection status of bluetooth input device.
     *
     * @returns true if bluetooth input device is connected
     */
    virtual bool isConnected() const = 0;

    /**
     * Retrieves MAC address of bluetooth input device.
     *
     * @returns MAC address of bluetooth input device
     */
    virtual QString address() const = 0;

    /**
     * Retrievies Name of bluetooth input device.
     *
     * @returns Name of bluetooth input device
     */
    virtual QString name() const = 0;

    /**
     * Retrieves Product ID of bluetooth input device.
     *
     * @returns Product ID of bluetooth input device
     */
    virtual QString productID() const = 0;

    /**
     * Retrieves Vendor ID of bluetooth input device.
     *
     * @returns Vendor ID of bluetooth input device
     */
    virtual QString vendorID() const = 0;

public Q_SLOTS:
    /**
     * Connect bluetooth input device.
     */
    virtual void slotConnect() = 0;

    /**
     * Disconnect bluetooth input device.
     */
    virtual void slotDisconnect() = 0;

Q_SIGNALS:
    /**
     * This signal is emitted when the bluetooth input device is connected.
     *
     * @param ubi the bluetooth input device identifier
     */
    virtual void connected() = 0;

    /**
     * This signal is emitted when the bluetooth input device is not available anymore.
     *
     * @param ubi the bluetooth input device identifier
     */
    virtual void disconnected() = 0;
};

} // Ifaces

} // Solid

Q_DECLARE_INTERFACE(SolidExperimental::Ifaces::BluetoothInputDevice, "org.kde.Solid.Ifaces.BluetoothInputDevice/0.1")

#endif
