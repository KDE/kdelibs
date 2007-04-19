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

#ifndef SOLID_IFACES_BLUETOOTHINTERFACE_H
#define SOLID_IFACES_BLUETOOTHINTERFACE_H

#include <QList>
#include <solid/solid_export.h>

#include <solid/experimental/bluetoothinterface.h>
#include <solid/experimental/ifaces/bluetoothremotedevice.h>

#include <QObject>

namespace SolidExperimental
{
namespace Ifaces
{
/**
 * Represents a bluetooth interface as seen by the bluetooth subsystem.
 */
class SOLIDIFACES_EXPORT BluetoothInterface : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a BluetoothInterface.
     *
     * @param parent the parent object
     */
    BluetoothInterface(QObject *parent = 0);

    /**
     * Destructs a BluetoothInterface object.
     */
    virtual ~BluetoothInterface();

    /**
     * Retrieves the Universal Bluetooth Identifier (UBI) of the BluetoothInterface.
     * This identifier is unique for each bluetooth remote device and bluetooth interface in the system.
     *
     * @returns the Universal Bluetooth Identifier of the current bluetooth interface
     */
    virtual QString ubi() const = 0;

    /**
     * Retrieves the MAC address of the bluetooth interface/adapter.
     *
     * @returns MAC address of bluetooth interface
     */
    virtual QString address() const = 0;

    /**
     * Retrieves the version of the chip of the bluetooth interface/adapter.
     * Example: "Bluetooth 2.0 + EDR"
     *
     * @returns bluetooth chip version
     */
    virtual QString version() const = 0;

    /**
     * Retrieves the revision of the chip of the bluetooth interface/adapter.
     * Example: "HCI 19.2"
     *
     * @returns bluetooth chip revision
     */
    virtual QString revision() const = 0;

    /**
     * Retrieves the name of the bluetooth chip manufacturer.
     * Example: "Cambdirge Silicon Radio"
     *
     * @returns manufacturer string of bluetooth interface/adapter
     */
    virtual QString manufacturer() const = 0;

    /**
     * Retrieves the name of the bluetooth chip company.
     * Based on device address.
     *
     * @returns company string of bluetooth interface/adapter
     */
    virtual QString company() const = 0;


    /**
     * Retrieves the current mode of the bluetooth interface/adapter.
     * Valid modes: "off", "connectable", "discoverable"
     *
     * @todo determine unify type for valid modes.. enum?! what about other bluetooth APIs?
     * three modes?
     *
     * @returns current mode of bluetooth interface/adaoter
     */
    virtual SolidExperimental::BluetoothInterface::Mode mode() const = 0;

    /**
     * Retrieves the discoverable timeout of the bluetooth interface/adapter.
     * Discoverable timeout of 0 means never disappear.
     *
     * @returns current discoverable timeout in seconds
     */
    virtual int discoverableTimeout() const = 0;

    /**
     * Retrieves the current discoverable staut of the bluetooth interface/adapter.
     *
     * @returns current discoverable status of bluetooth interface/adapter
     */
    virtual bool isDiscoverable() const = 0;


    /**
     * List all UBIs of connected remote bluetooth devices of this handled bluetooth
     * interface/adapter.
     *
     * @returns list UBIs of connected bluetooth remote devices
     */
    virtual QStringList listConnections() const = 0;


    /**
     * Retrieves major class of the bluetooth interface/adapter.
     *
     * @returns current major class of the bluetooth interface/adapter
     */
    virtual QString majorClass() const = 0;

    /**
     * List supported minor classes of the bluetooth interface/adapter.
     *
     * @returns list of supported minor classes by bluetooth interface/adapter
     */
    virtual QStringList listAvailableMinorClasses() const = 0;

    /**
     * Retrievies minor class of the bluetooth interface/adapter.
     * Valid classes, see listAvailableMinorClasses()
     *
     * @returns minor class of the bluetooth interface/adapter.
     */
    virtual QString minorClass() const = 0;

    /**
     * List services class of the bluetooth interface/adapter.
     *
     * @returns list of service classes or empty list if no services registered
     */
    virtual QStringList serviceClasses() const = 0;
    /**
     * Retrieves name of bluetooth interface/adapter.
     *
     * @returns name of bluetooth interface/adapter
     */
    virtual QString name() const = 0;

    /**
     * List UBIs of bonded/paired remote bluetooth devices with this bluetooth
     * interface/adapter.
     *
     * @returns UBIs of bonded/paired bluetooth remote devices
     */
    virtual QStringList listBondings() const = 0;

    /**
     * Periodic discovery status of this bluetooth interface/adapter.
     *
     * @returns true if periodic discovery is already active otherwise false
     */
    virtual bool isPeriodicDiscoveryActive() const = 0;

    /**
     * Name resolving status of periodic discovery routing. 
     *
     * @returns true if name got resolved while periodic discovery of this bluetooth
     * interface/adapter
     */
    virtual bool isPeriodicDiscoveryNameResolvingActive() const = 0;

    /**
     * List the Universal Bluetooth Identifier (UBI) of all known remote devices, which are
     * seen, used or paired/bonded.
     *
     * See listConnections()
     *
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
    virtual QStringList listRemoteDevices() const = 0;

    /**
     * List the Universal Bluetooth Identifier (UBI) of all known remote devices since a specifc
     * datestamp. Known remote  devices means remote bluetooth which are seen, used or
     * paired/bonded.
     *
     * See listConnections(), listRemoteDevices()
     *
     * @param date the datestamp of the beginning of recent used devices
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
    virtual QStringList listRecentRemoteDevices(const QDateTime &date) const = 0;

public Q_SLOTS:
    /**
     * Set mode of bluetooth interface/adapter.
     * Valid modes, see mode()
     *
     * @param mode the mode of the bluetooth interface/adapter
     */
    virtual void setMode(const SolidExperimental::BluetoothInterface::Mode mode) = 0;

    /**
     * Set discoverable timeout of bluetooth interface/adapter.
     *
     * @param timeout timeout in seconds
     */
    virtual void setDiscoverableTimeout(int timeout) = 0;

    /**
     * Set minor class of bluetooth interface/adapter.
     *
     * @param minor set minor class. Valid mode see listAvaliableMinorClasses()
     */
    virtual void setMinorClass(const QString &minor) = 0;

    /**
     * Set name of bluetooth interface/adapter.
     *
     * @param name the name of bluetooth interface/adapter
     */
    virtual void setName(const QString &name) = 0;


    /**
     * Start discovery of remote bluetooth devices with device name resolving.
     */
    virtual void discoverDevices() = 0;
    /**
     * Start discovery of remote bluetooth devices without device name resolving.
     */
    virtual void discoverDevicesWithoutNameResolving() = 0;
    /**
     * Cancel discovery of remote bluetooth devices.
     */
    virtual void cancelDiscovery() = 0;

    /**
     * Start periodic discovery of remote bluetooth devices.
     * See stopPeriodicDiscovery()
     */
    virtual void startPeriodicDiscovery() = 0;

    /**
     * Stop periodic discovery of remote bluetooth devices.
     */
    virtual void stopPeriodicDiscovery() = 0;

    /**
     * Enable/Disable name resolving of remote bluetooth devices in periodic discovery.
     *
     * @param resolveName true to enable name resolving otherwise false
     */
    virtual void setPeriodicDiscoveryNameResolving(bool resolveNames) = 0;

    /**
     * Instantiates a new BluetoothRemoteDevice object from this backend given its UBI.
     *
     * @param ubi the identifier of the bluetooth remote device instantiated
     * @returns a new BluetoothRemoteDevice object if there's a device having the given UBI, 0 otherwise
     */
    virtual QObject *createBluetoothRemoteDevice(const QString &ubi) = 0;

Q_SIGNALS:

    /**
     * This signal is emitted if the mode of the bluetooth interface/adapter has changed.
     * See mode() for valid modes.
     *
     * @param mode the changed mode
     */
    void modeChanged( SolidExperimental::BluetoothInterface::Mode );

    /**
     * The signal is emitted if the discoverable timeout of the bluetooth interface/adapter
     * has changed.
     *
     * @param timeout the changed timeout in seconds
     */
    void discoverableTimeoutChanged(int timeout);

    /**
     * The signal is emitted if the minor class of the bluetooth interface/adapter has changed.
     *
     * @param minor the new minor class
     */
    void minorClassChanged(const QString &minor);

    /**
     * The signal is emitted if the name of the bluetooth interface/adapter has changed.
     *
     * @param name the new name of the device
     */
    void nameChanged(const QString &name);

    /**
     * This signal is emitted if a discovery has started.
     */
    void discoveryStarted();

    /**
     * This signal is emitted if a discovery has completed.
     */
    void discoveryCompleted();

    /**
     * This signal is emitted if the bluetooth interface/adapter detects a new remote bluetooth device.
     *
     * @todo change arguments types of deviceClass (uint32) and rssi (int16)
     *
     * @param ubi the new bluetooth identifier
     * @param deviceClass the device Class of the remote device
     * @param rssi the RSSI link of the remote device
     */
    void remoteDeviceFound(const QString &ubi, int deviceClass, int rssi);

    /**
     * This signal is emitted if the bluetooth interface/adapter detectes a bluetooth device
     * disappeared.
     *
     * @param ubi the ubi of the disappering bluetooth remote device
     */
    void remoteDeviceDisappeared(const QString &ubi);

};
} //Ifaces
} //Solid

Q_DECLARE_INTERFACE(SolidExperimental::Ifaces::BluetoothInterface, "org.kde.Solid.Ifaces.BluetoothInterface/0.1")

#endif
