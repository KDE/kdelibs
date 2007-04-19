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

#ifndef SOLID_BLUETOOTHINPUTDEVICE_H
#define SOLID_BLUETOOTHINPUTDEVICE_H

#include <QtCore/QObject>

namespace SolidExperimental
{
class BluetoothInputDevicePrivate;

/**
 * Represents a bluetooth remote device as seen by the bluetoothing subsystem.
 */
class SOLID_EXPORT BluetoothInputDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new BluetoothInputDevice object.
     *
     * @param backendObject the bluetooth remote device object provided by the backend
     */
    BluetoothInputDevice(QObject *backendObject = 0);

    /**
     * Constructs a copy of a bluetooth remote device.
     *
     * @param device the bluetooth remote device to copy
     */
    BluetoothInputDevice(const BluetoothInputDevice &device);

    /**
     * Destroys the device.
     */
    ~BluetoothInputDevice();

    /**
     * Assigns a bluetooth remote device to this bluetooth remote device and returns a reference to it.
     *
     * @param device the bluetooth remote device to assign
     * @return a reference to the bluetooth remote device
     */
    BluetoothInputDevice &operator=(const BluetoothInputDevice &device);

    /**
     * Retrieves the Universal Network Identifier (UBI) of the BluetoothInputDevice.
     * This identifier is ubique for each bluetooth and bluetooth remote device in the system.
     *
     * @returns the Universal Network Identifier of the current bluetooth remote device
     */
    QString ubi() const;

    /**
     * Retrieves connection status of bluetooth input device.
     *
     * @returns true if bluetooth input device is connected
     */
    bool isConnected() const;

    /**
     * Retrieves MAC address of bluetooth input device.
     *
     * @returns MAC address of bluetooth input device
     */
    QString address() const;

    /**
     * Retrievies Name of bluetooth input device.
     *
     * @returns Name of bluetooth input device
     */
    QString name() const;

    /**
     * Retrieves Product ID of bluetooth input device.
     *
     * @returns Product ID of bluetooth input device
     */
    QString productID() const;

    /**
     * Retrieves Vendor ID of bluetooth input device.
     *
     * @returns Vendor ID of bluetooth input device
     */
    QString vendorID() const;


public Q_SLOTS:
    /**
     * Connect bluetooth input device.
     */
    void slotConnect();

    /**
     * Disconnect bluetooth input device.
     */
    void slotDisconnect();


Q_SIGNALS:
    /**
     * This signal is emitted when the bluetooth input device is connected.
     */
    void connected();

    /**
     * This signal is emitted when the bluetooth input device is not available anymore.
     */
    void disconnected();

private:
    Q_PRIVATE_SLOT(d, void _k_destroyed(QObject*))

    BluetoothInputDevicePrivate * const d;
};

} //Solid

//Q_DECLARE_OPERATORS_FOR_FLAGS( SolidExperimental::BluetoothInputDevice::Capabilities )

#endif
