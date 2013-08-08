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

#include <QDebug>

#include <solid/devicenotifier.h>
#include <solid/device.h>

SolidDeviceNotifier::SolidDeviceNotifier(QObject * parent)
    : QObject(parent)
    , m_notifier(Solid::DeviceNotifier::instance())
    , m_predicate(Solid::Predicate::fromString(query))
{
}

void SolidDeviceNotifier::addDevice(const QString & udi)
{
    if (m_predicate.matches(Solid::Device(udi))) {
        m_devices << udi;
        emit deviceAdded(udi);
        emitChange();
    }
}

void SolidDeviceNotifier::removeDevice(const QString & udi)
{
    if (m_devices.contains(udi)) {
        m_devices.removeAll(udi);
        emit deviceRemoved(udi);
        emitChange();
    }
}

int SolidDeviceNotifier::count() const
{
    return m_devices.count();
}

QStringList SolidDeviceNotifier::devices() const
{
    return m_devices;
}

void SolidDeviceNotifier::emitChange() const
{
    emit countChanged(m_devices.count());
    emit devicesChanged(m_devices);
}

void SolidDeviceNotifier::initialize()
{

}

QString SolidDeviceNotifier::query() const
{
    return m_query;
}

void SolidDeviceNotifier::setQuery(const QString & query)
{

}


#include "devicenotifier.moc"

