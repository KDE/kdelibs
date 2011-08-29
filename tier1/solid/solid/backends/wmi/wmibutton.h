/*
    Copyright 2006 Davide Bettio <davbet@aliceposta.it>

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

#ifndef SOLID_BACKENDS_WMI_BUTTON_H
#define SOLID_BACKENDS_WMI_BUTTON_H

#include <solid/ifaces/button.h>
#include "wmideviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace Wmi
{
class WmiDevice;

class Button : public DeviceInterface, virtual public Solid::Ifaces::Button
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Button)

public:
    Button(WmiDevice *device);
    virtual ~Button();

    virtual Solid::Button::ButtonType type() const;
    virtual bool hasState() const;
    virtual bool stateValue() const;

Q_SIGNALS:
    void pressed(Solid::Button::ButtonType type, const QString &udi);

private Q_SLOTS:
    void slotConditionRaised(const QString &condition, const QString &reason);
};
}
}
}

#endif // SOLID_BACKENDS_WMI_BUTTON_H
