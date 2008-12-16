/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davide.bettio@kdemail.net>

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

#ifndef SOLID_BACKENDS_FAKEHW_FAKEBUTTON_H
#define SOLID_BACKENDS_FAKEHW_FAKEBUTTON_H

#include "fakedeviceinterface.h"
#include <solid/ifaces/button.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeButton : public FakeDeviceInterface, virtual public Solid::Ifaces::Button
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Button)

public:
    explicit FakeButton(FakeDevice *device);
    ~FakeButton();

public Q_SLOTS:
    virtual Solid::Button::ButtonType type() const;
    virtual bool hasState() const;
    virtual bool stateValue() const;

    void press();

Q_SIGNALS:
    void pressed(Solid::Button::ButtonType type, const QString &udi);
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEBUTTON_H
