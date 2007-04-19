/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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

#ifndef SOLID_BLUETOOTHINTERFACE_H
#define SOLID_BLUETOOTHINTERFACE_H

#include <QDateTime>
#include <QPair>
#include <QtCore/QObject>

#include <solid/experimental/bluetoothremotedevice.h>
#include <solid/experimental/ifaces/bluetoothremotedevice.h>



namespace SolidExperimental
{
class BluetoothRemoteDevice;
typedef QList<BluetoothRemoteDevice> BluetoothRemoteDeviceList;
class BluetoothInterfacePrivate;

/**
 * Represents a bluetooth interface as seen by the bluetooth subsystem.
 */
class SOLID_EXPORT BluetoothInterface : public QObject
{
    Q_OBJECT

public:
    /**
     * Describes the operating mode of a bluetooth interface
     *
     * - Off : The interface's transceiver is turned off
     * - Discoverable : The interface may be discovered by other devices and connected to
     * - Connectable : The interface may only be connected to but not discovered
     */
    enum Mode { Off, Discoverable, Connectable };
    /**
     * Constructs an invalid bluetooth interface
     */
    BluetoothInterface();

    /**
     * Constructs a bluetooth interface for a given Unique Bluetooth Identifier (UBI).
     *
     * @param ubi the ubi of the bluetooth interface to create
     */
    explicit BluetoothInterface(const QString &ubi);

    /**
     * Constructs a new bluetooth interface taking its data from a backend.
     *
     * @param backendObject the object given by the backend
     */
    explicit BluetoothInterface(QObject *backendObject);

    /**
     * Constructs a copy of a bluetooth interface.
     *
     * @param device the bluetooth interface to copy
     */
    BluetoothInterface(const BluetoothInterface &device);

    /**
     * Destroys the device.
     */
    ~BluetoothInterface();


    /**
     * Assigns a bluetooth interface to this bluetooth interface and returns a reference to it.
     *
     * @param device the bluetooth interface to assign
     * @return a reference to the bluetooth interface
     */
    BluetoothInterface &operator=(const BluetoothInterface &device);

    /**
     * Retrieves the Unique Bluetooth Identifier (UBI) of the BluetoothInterface.
     * This identifier is ubique for each bluetooth and bluetooth interface in the system.
     *
     * @returns the Unique Bluetooth Identifier of the current bluetooth interface
     */
    QString ubi() const;

    /**
     * Create new BluetoothRemoteDevice object from this interface given its UBI.
     *
     * @param ubi the identifier of the bluetooth device to instantiate
     * @returns a bluetooth object, if a bluetooth device having the given UBI, for this interface exists, 0 otherwise
     */
    BluetoothRemoteDevice *createBluetoothRemoteDevice(const QString & ubi);

    /**
     * Finds a BluetoothRemoteDevice object given its UBI.
     *
     * @param ubi the identifier of the bluetooth remote device to find from this bluetooth interface
     * @returns a valid BluetoothRemoteDevice object if a remote device having the given UBI for this interface exists, an invalid BluetoothRemoteDevice object otherwise.
     */
    const BluetoothRemoteDevice &findBluetoothRemoteDevice(const QString &ubi) const;

    /**
     * Retrieves the MAC address of the bluetooth interface/adapter.
     *
     * @returns MAC address of bluetooth interface
     */
    QString address() const;

    /**
     * Retrieves the version of the chip of the bluetooth interface/adapter.
     * Example: "Bluetooth 2.0 + EDR"
     *
     * @returns bluetooth chip version
     */
    QString version() const;

    /**
     * Retrieves the revision of the chip of the bluetooth interface/adapter.
     * Example: "HCI 19.2"
     *
     * @returns bluetooth chip revision
     */
    QString revision() const;

    /**
     * Retrieves the name of the bluetooth chip manufacturer.
     * Example: "Cambridge Silicon Radio"
     *
     * @returns manufacturer string of bluetooth interface/adapter
     */
    QString manufacturer() const;

    /**
     * Retrieves the name of the manufacturer of the bluetooth interface,
     * using the chip supplied by BluetoothInterface::manufacterer()
     * Based on device address.
     *
     * @returns company string of bluetooth interface/adapter
     */
    QString company() const;

    /**
     * Retrieves the current mode of the bluetooth interface/adapter.
     *
     * @returns the current mode of bluetooth interface/adapter
     */
    Mode mode() const;

    /**
     * Retrieves the discoverable timeout of the bluetooth interface/adapter.
     * Discoverable timeout of 0 means never disappear.
     *
     * @returns current discoverable timeout in seconds
     */
    int discoverableTimeout() const;

    /**
     * Retrieves the current discoverable staut of the bluetooth interface/adapter.
     *
     * @returns current discoverable status of bluetooth interface/adapter
     */
    bool isDiscoverable() const;

    /**
     * List all UBIs of connected remote bluetooth devices of this handled bluetooth
     * interface/adapter.
     *
     * @returns list UBIs of connected bluetooth remote devices
     */
    BluetoothRemoteDeviceList listConnections() const;

    /**
     * Retrieves major class of the bluetooth interface/adapter.
     * @todo enum
     * @returns current major class of the bluetooth interface/adapter
     */
    QString majorClass() const;

    /**
     * List supported minor classes of the bluetooth interface/adapter.
     * @todo enum
     *
     * @returns list of supported minor classes by bluetooth interface/adapter
     */
    QStringList listAvailableMinorClasses() const;

    /**
     * Retrieves minor class of the bluetooth interface/adapter.
     * Valid classes, see listAvailableMinorClasses()
     * @todo enum
     *
     * @returns minor class of the bluetooth interface/adapter.
     */
    QString minorClass() const;

    /**
     * List services class of the bluetooth interface/adapter.
     * @todo enum
     *
     * @returns list of service classes or empty list if no services registered
     */
    QStringList serviceClasses() const;

    /**
     * Retrieves name of bluetooth interface/adapter.
     * @todo enum
     *
     * @returns name of bluetooth interface/adapter
     */
    QString name() const;

    /**
     * List UBIs of bonded/paired remote bluetooth devices with this bluetooth
     * interface/adapter.
     *
     * @returns UBIs of bonded/paired bluetooth remote devices
     */
    QStringList listBondings() const;

    /**
     * Periodic discovery status of this bluetooth interface/adapter.
     *
     * @returns true if periodic discovery is already active otherwise false
     */
    bool isPeriodicDiscoveryActive() const;

    /**
     * Name resolving status of periodic discovery routing.
     *
     * @returns true if name got resolved while periodic discovery of this bluetooth
     * interface/adapter
     */
    bool isPeriodicDiscoveryNameResolvingActive() const;

    /**
     * List the Unique Bluetooth Identifier (UBI) of all known remote devices, 
     * whether they are seen, used or paired/bonded.
     *
     * See listConnections()
     *
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
    QStringList listRemoteDevices() const;

    /**
     * List the Unique Bluetooth Identifier (UBI) of all known remote devices since a specific
     * datestamp. Known remote devices means remote bluetooth which are seen, used or
     * paired/bonded.
     *
     * See listConnections(), listRemoteDevices()
     *
     * @param date the datestamp of the beginning of recent used devices
     * @returns a QStringList of UBIs of all known remote bluetooth devices
     */
    QStringList listRecentRemoteDevices(const QDateTime &date) const;

public Q_SLOTS:
    /**
     * Set mode of bluetooth interface/adapter.
     * Valid modes, see mode()
     *
     * @param mode the mode of the bluetooth interface/adapter
     */
    void setMode(const Mode mode);

    /**
     * Set discoverable timeout of bluetooth interface/adapter.
     *
     * @param timeout timeout in seconds
     */
    void setDiscoverableTimeout(int timeout);

    /**
     * Set minor class of bluetooth interface/adapter.
     *
     * @param minor set minor class. Valid mode see listAvaliableMinorClasses()
     */
    void setMinorClass(const QString &minor);

    /**
     * Set name of bluetooth interface/adapter.
     *
     * @param name the name of bluetooth interface/adapter
     */
    void setName(const QString &name);

    /**
     * Start discovery of remote bluetooth devices with device name resolving.
     */
    void discoverDevices();
    /**
     * Start discovery of remote bluetooth devices without device name resolving.
     */
    void discoverDevicesWithoutNameResolving();
    /**
     * Cancel discovery of remote bluetooth devices.
     */
    void cancelDiscovery();

    /**
     * Start periodic discovery of remote bluetooth devices.
     * See stopPeriodicDiscovery()
     */
    void startPeriodicDiscovery();

    /**
     * Stop periodic discovery of remote bluetooth devices.
     */
    void stopPeriodicDiscovery();

    /**
     * Enable/Disable name resolving of remote bluetooth devices in periodic discovery.
     *
     * @param resolveName true to enable name resolving otherwise false
     */
    void setPeriodicDiscoveryNameResolving(bool resolveNames);

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
     * @param deviceClass the device class of the remote device
     * @param rssi the Received Signal Strength Information (RSSI) of the remote device
     */
    void remoteDeviceFound(const QString &ubi, int deviceClass, int rssi);

    /**
     * This signal is emitted if the bluetooth interface/adapter detectes a bluetooth device
     * disappeared.
     *
     * @param ubi the ubi of the disappering bluetooth remote device
     */
    void remoteDeviceDisappeared(const QString &ubi);

private:
    Q_PRIVATE_SLOT(d, void _k_destroyed(QObject*))

    BluetoothInterfacePrivate * const d;
};

} //Solid

#endif

