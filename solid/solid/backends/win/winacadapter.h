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
#ifndef WINACADAPTER_H
#define WINACADAPTER_H

#include <solid/ifaces/acadapter.h>
#include "wininterface.h"

namespace Solid
{
namespace Backends
{
namespace Win
{
class WinAcadapter : public WinInterface, virtual public Solid::Ifaces::AcAdapter
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::AcAdapter)
public:
    WinAcadapter(WinDevice *device);

    virtual bool isPlugged() const;

    static QSet<QString> getUdis();


signals:
    void plugStateChanged(bool newState, const QString &udi);

private slots:
    void powerChanged();

private:
    bool m_pluggedIn;

};

}
}
}
#endif // WINACADAPTER_H
