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

#ifndef SOLID_IFACES_BLUETOOTHREMOTEDEVICE
#define SOLID_IFACES_BLUETOOTHREMOTEDEVICE

#include <QObject>
#include <solid/solid_export.h>

class KJob;

namespace SolidExperimental
{
namespace Ifaces
{
/**
 * This interface represents a remote bluetooth device which we may be connected to.
 */
class SOLIDIFACES_EXPORT BluetoothRemoteDevice : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a BluetoothRemoteDevice.
     *
     * @param parent the parent object
     */
    BluetoothRemoteDevice(QObject *parent = 0);

    /**
     * Destructs a BluetoothRemoteDevice object.
     */
    virtual ~BluetoothRemoteDevice();

    /**
     * Retrieves the Universal Bluetooth Identifier (UBI) of the remote device.
     * This identifier is unique for each remote device and bluetooth interface in the system.
     *
     * @returns the Universal Bluetooth Identifier of the current remote device.
     */
    virtual QString ubi() const = 0;

    /**
     * Retrieves MAC address of the bluetooth remote device.
     *
     * @returns MAC address of remote device.
     */
    virtual QString address() const = 0;


    /**
     * Retrieves true if remote device is connected.
     *
     * @returns true if remote bluetooth device is connected otherwise false.
     */
    virtual bool isConnected() const = 0;

    /**
     * Retrieves the bluetooth version of the remote device.
     * LMP version (+ EDR support)
     *
     * @returns version of bluetooth chip.
     */
    virtual QString version() const = 0;

    /**
     * Retrieves the revision of the bluetooth chip of the remote device.
     *
     * @returns revision of bluetooth chip.
     */
    virtual QString revision() const = 0;

    /**
     * Retrieves company name based on the device address.
     *
     * @returns manufacturer string of bluetooth chip.
     */
    virtual QString manufacturer() const = 0;

    /**
     * Retrieves the manufacturer of the bluetooth chip of the remote device.
     *
     * @returns company string of the bluetooth chip.
     */
    virtual QString company() const = 0;

    /**
     * Retrieves the major class of the remote device.
     * Example: "computer"
     *
     * @returns major class of remote device.
     */
    virtual QString majorClass() const = 0;

    /**
     * Retrieves the minor class of the remote device.
     * Exampe: "laptop"
     *
     * @returns minor class of the remote device.
     */
    virtual QString minorClass() const = 0;

    /**
     * Retrieves a list of service classes of the remote device.
     * Example: ["networking", "object transfer" ]
     *
     * @returns list of service classes of the remote device.
     */
    virtual QStringList serviceClasses() const = 0;

    /**
     * Retrieves the real name of the remote device. See also alias().
     * Example: "Daniel's mobile"
     *
     * @returns name of remote device.
     */
    virtual QString name() const = 0;

    /**
     * Retrieves alias of remote device. This is a local alias name for the remote device.
     * If this string is empty the frontend should should use name(). This is handy if
     * someone is using several bluetooth remote device with the same name. alias() should
     * be prefered used by the frontend.
     * Example: "Company mobile"
     *
     * @retuns local alias of remote device.
     */
    virtual QString alias() const = 0;

    /**
     * Retrieves the date and time when the remote device has been seen.
     * Example: "2007-03-20 22:14:00 GMT"
     *
     * @returns date and time when the remote device has been seen.
     */
    virtual QString lastSeen() const = 0;

    /**
     * Retrieves the date and time when the remote device has been used.
     * Example: "2007-03-20 22:14:00 GMT"
     *
     * @returns date and time when the remote device has been used.
     */
    virtual QString lastUsed() const = 0;

    /**
     * Retrieves true if remote device has bonding.
     *
     * @returns true if remote device has bonding.
     */
    virtual bool hasBonding() const = 0;

    /**
     * Retrieves PIN code length that was used in the pairing process of remote device.
     *
     * @returns PIN code length of pairing.
     */
    virtual int pinCodeLength() const = 0;

    /**
     * Retrieves currently used encryption key size of remote device.
     *
     * @returns encryption key size.
     */
    virtual int encryptionKeySize() const = 0;

    /**
     * Create bonding ("pairing") with remote device.
     *
     * @returns the job handling of the operation.
     */
    virtual KJob *createBonding() = 0;


public Q_SLOTS:
    /**
     * Set alias for remote device.
     *
     * @param alias new alias name
     */
    virtual void setAlias(const QString &alias) = 0;

    /**
     * Clear alias for remote device.
     */
    virtual void clearAlias() = 0;

    /**
     * Disconnect remote device.
     */
    virtual void disconnect() = 0;

    /**
     * Cancel bonding process of remote device.
     */
    virtual void cancelBondingProcess() = 0;

    /**
     * Remove bonding bonding of remote device.
     */
    virtual void removeBonding() = 0;


Q_SIGNALS:
    /**
     * Class has been changed of remote device.
     *
     * @params deviceClass the device class of the remote device
     */
    virtual void classChanged(uint deviceClass) = 0;

    /**
     * Name has beend changed of remote device.
     *
     * @params name the name of the remote device
     */
    virtual void nameChanged(const QString &name) = 0;

    /**
     * Resolving of remote device name failed.
     */
    virtual void nameResolvingFailed() = 0;

    /**
     * Alias has been changed of remote device.
     *
     * @params alias the alias of the remote device
     */
    virtual void aliasChanged(const QString &alias) = 0;

    /**
     * Alias got cleared of remote device.
     */
    virtual void aliasCleared() = 0;

    /**
     * Remote device has been connected.
     */
    virtual void connected() = 0;

    /**
     * Disconnection has been requested for remote device.
     */
    virtual void requestDisconnection() = 0;

    /**
     * Remote device has been disconnected.
     */
    virtual void disconnected() = 0;

    /**
     * Bonding with remote device has been created.
     */
    virtual void bondingCreated() = 0;

    /**
     * Bonding has been removed of remote device.
     */
    virtual void bondingRemoved() = 0;

};

} // Ifaces

} // Solid

Q_DECLARE_INTERFACE(SolidExperimental::Ifaces::BluetoothRemoteDevice, "org.kde.Solid.Ifaces.BluetoothRemoteDevice/0.1")

#endif
