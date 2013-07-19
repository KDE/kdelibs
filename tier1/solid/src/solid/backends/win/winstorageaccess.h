/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WINSTORAGEACCESS_H
#define WINSTORAGEACCESS_H

#include <solid/ifaces/storageaccess.h>

#include "windevice.h"
#include "wininterface.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinStorageAccess : public WinInterface,  public virtual Solid::Ifaces::StorageAccess
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::StorageAccess)
public:
    WinStorageAccess(WinDevice *device);
    ~WinStorageAccess();

    virtual bool isAccessible() const;


    virtual QString filePath() const;

    virtual bool isIgnored() const;

    virtual bool setup();

    virtual bool teardown();

Q_SIGNALS:
    void accessibilityChanged(bool accessible, const QString &udi);

    void setupDone(Solid::ErrorType error, QVariant resultData, const QString &udi);

    void teardownDone(Solid::ErrorType error, QVariant resultData, const QString &udi);

    void setupRequested(const QString &udi);

    void teardownRequested(const QString &udi);
};
}
}
}


#endif // WINSTORAGEACCESS_H
