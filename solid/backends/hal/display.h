/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>

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

#ifndef DISPLAY_H
#define DISPLAY_H

#include <solid/ifaces/display.h>
#include "deviceinterface.h"

class HalDevice;

class Display : public DeviceInterface, virtual public Solid::Ifaces::Display
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Display)

public:
    Display(HalDevice *device);
    virtual ~Display();

    virtual Solid::Display::DisplayType type() const;
    virtual int lcdBrightness() const;
};

#endif
