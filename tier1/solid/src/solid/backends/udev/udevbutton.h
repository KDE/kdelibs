/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This library is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU Library General Public                      *
 *  License as published by the Free Software Foundation; either                     *
 *  version 2 of the License, or (at your option) any later version.                 *
 *                                                                                   *
 *  This library is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU                *
 *  Library General Public License for more details.                                 *
 *                                                                                   *
 *  You should have received a copy of the GNU Library General Public License        *
 *  along with this library; see the file COPYING.LIB.  If not, write to             *
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,             *
 *  Boston, MA 02110-1301, USA.                                                      *
 *************************************************************************************/

#ifndef SOLID_BACKENDS_UDEV_BUTTON_H
#define SOLID_BACKENDS_UDEV_BUTTON_H

#include <solid/ifaces/button.h>
#include "udevdeviceinterface.h"

namespace Solid
{
namespace Backends
{
namespace UDev
{
class Button : public DeviceInterface, virtual public Solid::Ifaces::Button
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Button)

public:
    Button(UDevDevice *device);
    virtual ~Button();

    virtual bool hasState() const;
    virtual bool stateValue() const;
    virtual Solid::Button::ButtonType type() const;

Q_SIGNALS:
    virtual void pressed(Solid::Button::ButtonType type, const QString& udi);

private:
    Solid::Button::ButtonType m_type;
};
}
}
}

#endif
