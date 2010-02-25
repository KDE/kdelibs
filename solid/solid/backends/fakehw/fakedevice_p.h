/*  This file is part of the KDE project
 *  Copyright (C) 2006 Michaël Larouche <michael.larouche@kdemail.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */
#ifndef SOLID_BACKENDS_FAKEHW_FAKEDEVICE_P_H
#define SOLID_BACKENDS_FAKEHW_FAKEDEVICE_P_H

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QObject>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDevice;
class FakeDevice::Private : public QObject
{
    Q_OBJECT
public:
    QString udi;
    QMap<QString, QVariant> propertyMap;
    QStringList interfaceList;
    bool locked;
    QString lockReason;
    bool broken;

Q_SIGNALS:
    void propertyChanged(const QMap<QString,int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);

    friend class FakeDevice;
};
}
}
}

#endif
