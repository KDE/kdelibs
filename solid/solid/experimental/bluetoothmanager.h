/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
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

#ifndef SOLID_BLUETOOTHMANAGER
#define SOLID_BLUETOOTHMANAGER

#include <QtCore/QObject>

#include <solid/singletondefs.h>
#include <solid/solid_export.h>

#include <solid/experimental/bluetoothinterface.h>
#include <solid/experimental/bluetoothinputdevice.h>

class KJob;

namespace SolidExperimental
{
namespace Ifaces
{
class BluetoothManager;
}

class BluetoothInterface;

typedef QList<BluetoothInterface> BluetoothInterfaceList;
typedef QList<BluetoothInputDevice> BluetoothInputDeviceList;
class BluetoothManagerPrivate;

/**
 * This class specifies the interface a backend will have to implement in
 * order to be used in the system.
 *
 * A bluetooth manager allow to query the underlying platform to discover the
 * available bluetooth interfaces.
 */
class SOLID_EXPORT BluetoothManager : public QObject
{
    Q_OBJECT
    SOLID_SINGLETON(BluetoothManager)

public:
    /**
     * Retrieves the list of all the bluetooth interfaces Universal Bluetooth Identifiers (UBIs)
     * in the system.
     *
     * @return the list of bluetooth interfaces available in this system
     */
    BluetoothInterfaceList bluetoothInterfaces() const;

    /**
     * Retrieves the default bluetooth interface Universal Bluetooth Identifiers (UBIs)
     * of the system.
     *
     * @return the UBI of the default bluetooth interface
     */
    QString defaultInterface() const;

    /**
     * Find a new BluetoothInterface object given its UBI.
     *
     * @param ubi the identifier of the bluetooth interface to find
     * @returns a valid BlueoothInterface object if there's a device having the given UBI, an invalid one otherwise
     */
    const BluetoothInterface &findBluetoothInterface(const QString & ubi) const;


    /**
     * Find a new BluetoothInputDevice object given its UBI.
     *
     * @param ubi the identifier of the bluetooth input device to find
     * @returns a valid BlueoothInputDevice object if there's a device having the given UBI, an invalid one otherwise
     */
    const BluetoothInputDevice &findBluetoothInputDevice(const QString & ubi) const;

    /**
     * Retrieves the list of Universal Bluetooth Identifiers (UBIs) of bluetooth input devices
     * which are configured in the system. Configured means also not connected devices.
     *
     * @return the list of bluetooth input devices configured in this system
     */
    BluetoothInputDeviceList bluetoothInputDevices() const;

    /**
     * Instantiates a new BluetoothInputDevice object from this backend given its UBI.
     *
     * @param ubi the identifier of the bluetooth input device instantiated
     * @returns a new BluetoothInputDevice object if there's a device having the given UBI, 0 otherwise
     */
    BluetoothInputDevice *createBluetoothInputDevice(const QString &ubi);

    /**
     * Setup a new bluetooth input device.
     *
     * @param ubi the ubi of the bluetooth input device
     * @returns the job handling of the operation.
     */
    KJob *setupInputDevice(const QString &ubi);

public Q_SLOTS:
    /**
     * Remove the configuraiton of a bluetooth input device.
     *
     * @param ubi the bluetooth input device identifier
     */
    void removeInputDevice(const QString & ubi);

Q_SIGNALS:
    /**
     * This signal is emitted when a new bluetooth interface is available.
     *
     * @param ubi the bluetooth interface identifier
     */
    void interfaceAdded(const QString & ubi);

    /**
     * This signal is emitted when a bluetooth interface is not available anymore.
     *
     * @param ubi the bluetooth interface identifier
     */
    void interfaceRemoved(const QString & ubi);

    /**
     * This signal is emitted when the default bluetooth interface changed.
     *
     * @param ubi the bluetooth interface identifier
     */
    void defaultInterfaceChanged(const QString & ubi);

    /**
     * This signal is emitted when a new bluetooth input device got configured/created.
     *
     * @param ubi the bluetooth input device identifier
     */
    void inputDeviceCreated(const QString & ubi);

    /**
     * This signal is emitted when a bluetooth input device configuration is not available anymore.
     *
     * @param ubi the bluetooth input device identifier
     */
    void inputDeviceRemoved(const QString & ubi);


private:
    BluetoothManager();
    ~BluetoothManager();

    BluetoothInterfaceList buildDeviceList(const QStringList & ubiList) const;

    Q_PRIVATE_SLOT(d, void _k_interfaceAdded(const QString&))
    Q_PRIVATE_SLOT(d, void _k_interfaceRemoved(const QString&))
    Q_PRIVATE_SLOT(d, void _k_interfaceDestroyed(QObject*))

    Q_PRIVATE_SLOT(d, void _k_inputDeviceCreated(const QString&))
    Q_PRIVATE_SLOT(d, void _k_inputDeviceRemoved(const QString&))
    Q_PRIVATE_SLOT(d, void _k_inputDeviceDestroyed(QObject*))

    BluetoothManagerPrivate * const d;
    friend class BluetoothManagerPrivate;
};
} // Solid

#endif
