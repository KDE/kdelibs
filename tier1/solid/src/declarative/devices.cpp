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

// Maps queries to the handler objects
QHash<QString, QWeakPointer<DevicesQueryPrivate> > DevicesQueryPrivate::handlers;

QSharedPointer<DevicesQueryPrivate> DevicesQueryPrivate::forQuery(const QString & query)
{
    if (handlers.contains(query)) {
        return handlers[query].toStrongRef();
    }

    // Creating a new shared backend instance
    QSharedPointer<DevicesQueryPrivate> backend(new DevicesQueryPrivate(query));

    // Storing a weak pointer to the backend
    handlers[query] = backend;

    // Returns the newly created backend
    // TODO: It would be nicer with std::move and STL's smart pointers,
    // but RVO should optimize this out.
    return backend;
}

DevicesQueryPrivate::DevicesQueryPrivate(const QString & query)
    : query(query)
    , predicate(Solid::Predicate::fromString(query))
    , notifier(Solid::DeviceNotifier::instance())
{
    connect(notifier, &Solid::DeviceNotifier::deviceAdded,
            this,     &DevicesQueryPrivate::addDevice);
    connect(notifier, &Solid::DeviceNotifier::deviceRemoved,
            this,     &DevicesQueryPrivate::removeDevice);

    if (!query.isEmpty() && !predicate.isValid()) return;

    Q_FOREACH(const Solid::Device & device, Solid::Device::listFromQuery(predicate)) {
        matchingDevices << device.udi();
    }
}

DevicesQueryPrivate::~DevicesQueryPrivate()
{
    handlers.remove(query);
}

void DevicesQueryPrivate::addDevice(const QString & udi)
{
    if (predicate.isValid() && predicate.matches(Solid::Device(udi))) {
        matchingDevices << udi;
        emit deviceAdded(udi);
    }
}

void DevicesQueryPrivate::removeDevice(const QString & udi)
{
    if (predicate.isValid() && matchingDevices.contains(udi)) {
        matchingDevices.removeAll(udi);
        emit deviceRemoved(udi);
    }
}

const QStringList & DevicesQueryPrivate::devices() const
{
    return matchingDevices;
}


DevicesPrivate::DevicesPrivate(Devices * parent)
    : q(parent)
{
}

void DevicesPrivate::initialize()
{
    if (backend) return;

    backend = DevicesQueryPrivate::forQuery(query);

    connect(backend.data(), &DevicesQueryPrivate::deviceAdded,
            this, &DevicesPrivate::addDevice);
    connect(backend.data(), &DevicesQueryPrivate::deviceRemoved,
            this, &DevicesPrivate::removeDevice);

    const int matchesCount = backend->devices().count();

    if (matchesCount != 0) {
        emit q->isEmptyChanged(false);
        emit q->countChanged(matchesCount);
        emit q->devicesChanged(backend->devices());
    }
}

void DevicesPrivate::reset()
{
    if (!backend) return;

    backend->disconnect(this);
    backend.reset();

    emit q->isEmptyChanged(true);
    emit q->countChanged(0);
    emit q->devicesChanged(QStringList());
}

void DevicesPrivate::addDevice(const QString & udi)
{
    if (!backend) return;

    const int count = backend->devices().count();

    if (count == 1) {
        emit q->isEmptyChanged(false);
    }

    emit q->countChanged(count);
    emit q->devicesChanged(backend->devices());
    emit q->deviceAdded(udi);
}

void DevicesPrivate::removeDevice(const QString & udi)
{
    if (!backend) return;

    const int count = backend->devices().count();

    if (count == 0) {
        emit q->isEmptyChanged(true);
    }

    emit q->countChanged(count);
    emit q->devicesChanged(backend->devices());
    emit q->deviceRemoved(udi);
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
    return count() == 0;
}

int Devices::count() const
{
    d->initialize();
    return devices().count();
}

QStringList Devices::devices() const
{
    d->initialize();
    return d->backend->devices();
}

QString Devices::query() const
{
    return d->backend->query;
}

void Devices::setQuery(const QString & query)
{
    if (d->query == query) return;

    d->query = query;

    d->reset();
    d->initialize();

    emit queryChanged(query);
}

QObject * Devices::device(const QString & udi, const QString & _type)
{
    Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(_type);

    return Solid::Device(udi).asDeviceInterface(type);
}
} // namespace Solid

#include "devices.moc"

