/*
    Copyright 2013 Ivan Cukic <ivan.cukic@kde.org>

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

#include "udevpointer.h"

#include <QDebug>

using namespace Solid::Backends::UDev;

Pointer::Pointer(UDevDevice *device)
    : DeviceInterface(device)
    , m_type(
          device->property("ID_INPUT_MOUSE").toInt()       == 1 ? Solid::Pointer::MousePointer :
          device->property("ID_INPUT_TOUCHPAD").toInt()    == 1 ? Solid::Pointer::TouchpadPointer :
          device->property("ID_INPUT_TABLET").toInt()      == 1 ? Solid::Pointer::TabletPointer :
          device->property("ID_INPUT_TOUCHSCREEN").toInt() == 1 ? Solid::Pointer::TouchscreenPointer :
                                                                  Solid::Pointer::UnknownPointerType
      )
{
    qDebug() << "This is the udev pointer type" << m_type;
}

Pointer::~Pointer()
{

}

Solid::Pointer::PointerType Pointer::type() const
{
    return m_type;
}

