/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_BACKENDS_FAKEHW_FAKESTORAGEACCESS_H
#define SOLID_BACKENDS_FAKEHW_FAKESTORAGEACCESS_H

#include <solid/ifaces/storageaccess.h>
#include "fakedeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeStorageAccess : public FakeDeviceInterface, virtual public Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)

public:
    FakeStorageAccess(FakeDevice *device);
    virtual ~FakeStorageAccess();

    virtual bool isAccessible() const;
    virtual QString filePath() const;
public Q_SLOTS:
    virtual bool setup();
    virtual bool teardown();

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi);
    void setupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void teardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
    void setupRequested(const QString &udi);
    void teardownRequested(const QString &udi);

private Q_SLOTS:
    void onPropertyChanged(const QMap<QString,int> &changes);
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKESTORAGEACCESS_H
