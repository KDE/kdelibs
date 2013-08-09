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

#include "devicenotifier.h"
#include "devicenotifier_p.h"

#include <QDebug>

#include <solid/devicenotifier.h>
#include <solid/device.h>

SolidDeviceNotifierPrivate::SolidDeviceNotifierPrivate(SolidDeviceNotifier * parent)
    : q(parent)
    , notifier(Solid::DeviceNotifier::instance())
    , initialized(false)
{
    connect(notifier, &Solid::DeviceNotifier::deviceAdded,
            this,     &SolidDeviceNotifierPrivate::addDevice);
    connect(notifier, &Solid::DeviceNotifier::deviceRemoved,
            this,     &SolidDeviceNotifierPrivate::removeDevice);
}

void SolidDeviceNotifierPrivate::addDevice(const QString & udi)
{
    if (!initialized) return;

    if (predicate.matches(Solid::Device(udi))) {
        devices << udi;
        emit q->deviceAdded(udi);
        emitChange();
    }
}

void SolidDeviceNotifierPrivate::removeDevice(const QString & udi)
{
    if (!initialized) return;

    if (devices.contains(udi)) {
        devices.removeAll(udi);
        emit q->deviceRemoved(udi);
        emitChange();
    }
}

void SolidDeviceNotifierPrivate::emitChange() const
{
    emit q->countChanged(devices.count());
    emit q->devicesChanged(devices);
}

void SolidDeviceNotifierPrivate::initialize()
{
    if (initialized) return;
    initialized = true;

    qDebug() << "This is the query to be used: " << query;
    predicate = Solid::Predicate::fromString(query);
    qDebug() << "Predicate: " << predicate.toString();

    Q_FOREACH(const Solid::Device & device, Solid::Device::listFromQuery(predicate)) {
        devices << device.udi();
    }
}

void SolidDeviceNotifierPrivate::reset()
{
    if (!initialized) return;
    initialized = false;
    devices.clear();
}

SolidDeviceNotifier::SolidDeviceNotifier(QObject * parent)
    : QObject(parent), d(new SolidDeviceNotifierPrivate(this))
{
}

SolidDeviceNotifier::~SolidDeviceNotifier()
{
    delete d;
}

int SolidDeviceNotifier::count() const
{
    d->initialize();
    return d->devices.count();
}

QStringList SolidDeviceNotifier::devices() const
{
    d->initialize();
    return d->devices;
}

QString SolidDeviceNotifier::query() const
{
    return d->query;
}

void SolidDeviceNotifier::setQuery(const QString & query)
{
    if (d->query == query) return;

    d->query = query;
    d->reset();

    emit queryChanged(query);
}

#include "devicenotifier.moc"

