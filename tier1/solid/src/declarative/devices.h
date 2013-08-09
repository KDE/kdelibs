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

#ifndef SOLID_DECALARATIVE_DEVICES_H
#define SOLID_DECALARATIVE_DEVICES_H

#include <QObject>
#include <solid/predicate.h>
#include <solid/deviceinterface.h>

namespace Solid {

class DeviceNotifier;
class DevicesPrivate;

/**
 * A class that watches the devices known to the solid system.
 *
 * It behaves similarly to Solid::DeviceNotifier, but
 * adds some convenience methods which allow it to
 * watch only the devices matching a specified query
 * (formatted for Solid::Predicate).
 *
 */
class Devices: public QObject {
    Q_OBJECT

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)

public:
    explicit Devices(QObject * parent = Q_NULLPTR);
    ~Devices();

Q_SIGNALS:
    void deviceAdded(const QString & udi) const;
    void deviceRemoved(const QString & udi) const;

    void countChanged(int count) const;
    void devicesChanged(const QStringList & devices) const;
    void queryChanged(const QString & query) const;
    void isEmptyChanged(bool empty) const;

public Q_SLOTS:
    int count() const;
    bool isEmpty() const;
    QStringList devices() const;

    QString query() const;
    void setQuery(const QString & query);

    QObject * device(const QString & udi, const QString & type);

private:
    friend class DevicesPrivate;
    DevicesPrivate * const d;
};

} // namespace Solid

#endif

