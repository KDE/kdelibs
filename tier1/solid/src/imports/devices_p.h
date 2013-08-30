/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SOLID_DECALARATIVE_DEVICE_NOTIFIER_P_H
#define SOLID_DECALARATIVE_DEVICE_NOTIFIER_P_H

#include "devices.h"

#include <QSharedPointer>
#include <QWeakPointer>

#include <solid/devicenotifier.h>
#include <solid/device.h>

namespace Solid {

/**
 * Instances of this class are used as backends for
 * Devices and DevicesPrivate classes.
 *
 * The purpose of it is to create only one filter
 * (Solid::Predicate) and the corresponding devices list
 * per query, because there can be multiple clients
 * interested in the same device types.
 *
 * The user of the class needs not to worry about object
 * allocation and deallocation - just use DevicesQueryPrivate::forQuery
 * and store the retrieved smart pointer.
 */
class DevicesQueryPrivate: public QObject {
    Q_OBJECT

public:
    /**
     * Returns a shared pointer to a handler for the specified query
     */
    static QSharedPointer<DevicesQueryPrivate> forQuery(const QString &query);

    ~DevicesQueryPrivate();

    /**
     * Returns a list of devices that match the query
     */
    const QStringList &devices() const;

    /**
     * A query which is used to create the predicate.
     * It can be public since it is immutable.
     */
    const QString query;

    /**
     * A predicate used for checking whether a device
     * satisfies the specified query.
     * It can be public since it is immutable.
     */
    const Solid::Predicate predicate;

Q_SIGNALS:
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);

public Q_SLOTS:
    void addDevice(const QString &udi);
    void removeDevice(const QString &udi);

private:
    DevicesQueryPrivate(const QString &query);

    // TODO: This could be static or something
    Solid::DeviceNotifier *const notifier;

    QStringList matchingDevices;

    // Maps queries to the handler objects
    static QHash<QString, QWeakPointer<DevicesQueryPrivate> > handlers;
};

} // namespace Solid

#endif

