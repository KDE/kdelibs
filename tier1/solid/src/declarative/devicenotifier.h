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

#ifndef SOLID_DECALARATIVE_DEVICE_NOTIFIER_H
#define SOLID_DECALARATIVE_DEVICE_NOTIFIER_H

#include <QObject>
#include <solid/predicate.h>

namespace Solid {
    class DeviceNotifier;
}

/**
 * A small interface class that allows the
 * indirect use of the DeviceNotifier singleton
 * inside of QML
 */
class SolidDeviceNotifier: public QObject {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTFIY queryChanged)

public:
    explicit SolidDeviceNotifier(
            const QString & query,
            QObject * parent = Q_NULLPTR);

Q_SIGNALS:
    void deviceAdded(const QString & udi) const;
    void deviceRemoved(const QString & udi) const;

    void countChanged(int count) const;
    void devicesChanged(const QStringList & devices) const;
    void queryChanged(const QString & query) const;

public:
    Q_INVOKABLE int count() const;
    Q_INVOKABLE QStringList devices() const;
    Q_INVOKABLE QString query() const;

private Q_SLOTS:
    void addDevice(const QString & udi);
    void removeDevice(const QString & udi);
    void setQuery(const QString & query);

private:
    void emitChange() const;

    void initQuery();

    Solid::DeviceNotifier * m_notifier;
    Solid::Predicate m_predicate;
    QStringList m_devices;
    bool m_initialized;

};

#endif

