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

#include "devices.h"
#include "devices_p.h"

#include <QDebug>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/genericinterface.h>

namespace Solid {

DevicesPrivate::DevicesPrivate(Devices * parent)
    : q(parent)
    , notifier(Solid::DeviceNotifier::instance())
    , initialized(false)
{
    connect(notifier, &Solid::DeviceNotifier::deviceAdded,
            this,     &DevicesPrivate::addDevice);
    connect(notifier, &Solid::DeviceNotifier::deviceRemoved,
            this,     &DevicesPrivate::removeDevice);
}

void DevicesPrivate::addDevice(const QString & udi)
{
    if (!initialized) return;

    if (predicate.matches(Solid::Device(udi))) {
        devices << udi;
        emit q->deviceAdded(udi);
        emitChange();

        if (devices.count() == 1) {
            emit q->isEmptyChanged(false);
        }
    }
}

void DevicesPrivate::removeDevice(const QString & udi)
{
    if (!initialized) return;

    if (devices.contains(udi)) {
        devices.removeAll(udi);
        emit q->deviceRemoved(udi);
        emitChange();

        if (devices.count() == 0) {
            emit q->isEmptyChanged(true);
        }
    }
}

void DevicesPrivate::emitChange() const
{
    emit q->countChanged(devices.count());
    emit q->devicesChanged(devices);
}

void DevicesPrivate::initialize()
{
    if (initialized) return;

    qDebug() << "This is the query to be used: " << query;
    predicate = Solid::Predicate::fromString(query);
    qDebug() << "Predicate: " << predicate.isValid() << " " << predicate.toString();

    if (!query.isEmpty() && !predicate.isValid()) return;

    initialized = true;

    Q_FOREACH(const Solid::Device & device, Solid::Device::listFromQuery(predicate)) {
        devices << device.udi();
    }

    if (devices.count()) {
        emit q->isEmptyChanged(false);
    }
}

void DevicesPrivate::reset()
{
    if (!initialized) return;
    initialized = false;
    devices.clear();
}

Devices::Devices(QObject * parent)
    : QObject(parent), d(new DevicesPrivate(this))
{
}

Devices::~Devices()
{
    delete d;
}

bool Devices::isEmpty() const
{
    d->initialize();
    return d->devices.count() == 0;
}

int Devices::count() const
{
    d->initialize();
    return d->devices.count();
}

QStringList Devices::devices() const
{
    d->initialize();
    return d->devices;
}

QString Devices::query() const
{
    return d->query;
}

void Devices::setQuery(const QString & query)
{
    if (d->query == query) return;

    d->query = query;
    d->reset();

    emit queryChanged(query);
}

QObject * Devices::device(const QString & udi, const QString & _type)
{
    Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(_type);

    return Solid::Device(udi).asDeviceInterface(type);
}

} // namespace Solid

#include "devices.moc"

