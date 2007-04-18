/*  This file is part of the KDE project
    Copyright (C) 2006 Davide Bettio <davbet@aliceposta.it>
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

#include "display.h"
#include "display_p.h"

#include "soliddefs_p.h"
#include <solid/ifaces/display.h>

Solid::Display::Display( QObject *backendObject )
    : Capability(*new DisplayPrivate(this), backendObject)
{
}

Solid::Display::~Display()
{

}

Solid::Display::DisplayType Solid::Display::type() const
{
    Q_D(const Display);
    return_SOLID_CALL(Ifaces::Display*, d->backendObject(), UnknownDisplayType, type());
}

int Solid::Display::lcdBrightness() const
{
    Q_D(const Display);
    return_SOLID_CALL(Ifaces::Display*, d->backendObject(), 100, lcdBrightness());
}

#include "display.moc"
