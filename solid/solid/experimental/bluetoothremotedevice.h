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

#ifndef SOLID_BLUETOOTHREMOTEDEVICE_H
#define SOLID_BLUETOOTHREMOTEDEVICE_H

#include <QtCore/QObject>

#include <solid/experimental/bluetoothmanager.h>
#include <solid/experimental/bluetoothinterface.h>

class KJob;

namespace SolidExperimental
{
class BluetoothRemoteDevicePrivate;

/**
 * Represents a bluetooth remote device as seen by the bluetoothing subsystem.
 */
class SOLID_EXPORT BluetoothRemoteDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(BluetoothRemoteDevice)

public:
    /**
     * Creates a new BluetoothRemoteDevice object.
     *
     * @param backendObject the bluetooth remote device object provided by the backend
     */
    BluetoothRemoteDevice(QObject *backendObject = 0);

    /**
     * Constructs a copy of a bluetooth remote device.
     *
     * @param device the bluetooth remote device to copy
     */
    BluetoothRemoteDevice(const BluetoothRemoteDevice &device);

    /**
     * Destroys the device.
     */
    ~BluetoothRemoteDevice();

    /**
     * Assigns a bluetooth remote device to this bluetooth remote device and returns a reference to it.
     *
     * @param device the bluetooth remote device to assign
     * @return a reference to the bluetooth remote device
     */
    BluetoothRemoteDevice &operator=(const BluetoothRemoteDevice &device);

    /**
     * Retrieves the Universal Bluetooth Identifier (UBI) of the remote device.
     * This identifier is unique for each remote device and bluetooth interface in the system.
     *
     * @returns the Universal Bluetooth Identifier of the current remote device.
     */
    QString ubi() const;

    /**
     * Retrieves MAC address of the bluetooth remote device.
     *
     * @returns MAC address of remote device.
     */
    QString address() const;


    /**
     * Retrieves true if remote device is connected.
     *
     * @returns true if remote bluetooth device is connected otherwise false.
     */
    bool isConnected() const;

    /**
     * Retrieves the bluetooth version of the remote device.
     * LMP version (+ EDR support)
     *
     * @returns version of bluetooth chip.
     */
    QString version() const;

    /**
     * Retrieves the revision of the bluetooth chip of the remote device.
     *
     * @returns revision of bluetooth chip.
     */
    QString revision() const;

    /**
     * Retrieves company name based on the device address.
     *
     * @returns manufacturer string of bluetooth chip.
     */
    QString manufacturer() const;

    /**
     * Retrieves the manufacturer of the bluetooth chip of the remote device.
     *
     * @returns company string of the bluetooth chip.
     */
    QString company() const;

    /**
     * Retrieves the major class of the remote device.
     * Example: "computer"
     *
     * @returns major class of remote device.
     */
    QString majorClass() const;

    /**
     * Retrieves the minor class of the remote device.
     * Exampe: "laptop"
     *
     * @returns minor class of the remote device.
     */
    QString minorClass() const;

    /**
     * Retrieves a list of service classes of the remote device.
     * Example: ["networking", "object transfer" ]
     *
     * @returns list of service classes of the remote device.
     */
    QStringList serviceClasses() const;

    /**
     * Retrieves the real name of the remote device. See also alias().
     * Example: "Daniel's mobile"
     *
     * @returns name of remote device.
     */
    QString name() const;

    /**
     * Retrieves alias of remote device. This is a local alias name for the remote device.
     * If this string is empty the frontend should should use name(). This is handy if
     * someone is using several bluetooth remote device with the same name. alias() should
     * be prefered used by the frontend.
     * Example: "Company mobile"
     *
     * @retuns local alias of remote device.
     */
    QString alias() const;

    /**
     * Retrieves the date and time when the remote device has been seen.
     * Example: "2007-03-20 22:14:00 GMT"
     *
     * @returns date and time when the remote device has been seen.
     */
    QString lastSeen() const;

    /**
     * Retrieves the date and time when the remote device has been used.
     * Example: "2007-03-20 22:14:00 GMT"
     *
     * @returns date and time when the remote device has been used.
     */
    QString lastUsed() const;

    /**
     * Retrieves true if remote device has bonding.
     *
     * @returns true if remote device has bonding.
     */
    bool hasBonding() const;

    /**
     * Retrieves PIN code length that was used in the pairing process of remote device.
     *
     * @returns PIN code length of pairing.
     */
    int pinCodeLength() const;

    /**
     * Retrieves currently used encryption key size of remote device.
     *
     * @returns encryption key size.
     */
    int encryptionKeySize() const;

    /**
     * Create bonding ("pairing") with remote device.
     *
     * @returns the job handling of the operation.
     */
    KJob *createBonding();

public Q_SLOTS:
    /**
     * Set alias for remote device.
     *
     * @param alias new alias name
     */
    void setAlias(const QString &alias);

    /**
     * Clear alias for remote device.
     */
    void clearAlias();

    /**
     * Disconnect remote device.
     */
    void disconnect();

    /**
     * Cancel bonding process of remote device.
     */
    void cancelBondingProcess();

    /**
     * Remove bonding bonding of remote device.
     */
    void removeBonding();


Q_SIGNALS:
    /**
     * Class has been changed of remote device.
     *
     * @params deviceClass the device class of the remote device
     */
    void classChanged(uint deviceClass);

    /**
     * Name has beend changed of remote device.
     *
     * @params name the name of the remote device
     */
    void nameChanged(const QString &name);

    /**
     * Resolving of remote device name failed.
     */
    void nameResolvingFailed();

    /**
     * Alias has been changed of remote device.
     *
     * @params alias the alias of the remote device
     */
    void aliasChanged(const QString &alias);

    /**
     * Alias got cleared of remote device.
     */
    void aliasCleared();

    /**
     * Remote device has been connected.
     */
    void connected();

    /**
     * Disconnection has been requested for remote device.
     */
    void requestDisconnection();

    /**
     * Remote device has been disconnected.
     */
    void disconnected();

    /**
     * Bonding with remote device has been created.
     */
    void bondingCreated();

    /**
     * Bonding has been removed of remote device.
     */
    void bondingRemoved();

protected:
    BluetoothRemoteDevicePrivate *d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _k_destroyed(QObject*))
};

} //Solid

#endif
