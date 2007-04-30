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

#include "backends/fakehw/fakedisplay.h"

FakeDisplay::FakeDisplay(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakeDisplay::~FakeDisplay()
{

}

Solid::Display::DisplayType FakeDisplay::type() const
{

    QString displaytype = fakeDevice()->property("type").toString();

    if (displaytype=="lcd")
    {
        return Solid::Display::Lcd;
    }
    else if (displaytype=="crt")
    {
        return Solid::Display::Crt;
    }
    else if (displaytype=="tv_out")
    {
        return Solid::Display::TvOut;
    }
    else
    {
        return Solid::Display::UnknownDisplayType;
    }
}

int FakeDisplay::lcdBrightness() const
{
    return fakeDevice()->property("lcdBrightness").toInt();
}

#include "backends/fakehw/fakedisplay.moc"
