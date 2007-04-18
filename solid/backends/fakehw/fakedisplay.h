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

#ifndef FAKEDISPLAY_H
#define FAKEDISPLAY_H

#include "fakedeviceinterface.h"
#include <solid/ifaces/display.h>

class FakeDisplay : public FakeDeviceInterface, public virtual Solid::Ifaces::Display
{
    Q_OBJECT
    Q_INTERFACES( Solid::Ifaces::Display )

public:
    explicit FakeDisplay(FakeDevice *device);
    ~FakeDisplay();

public Q_SLOTS:
    virtual Solid::Display::DisplayType type() const;
    virtual int lcdBrightness() const;
};

#endif
