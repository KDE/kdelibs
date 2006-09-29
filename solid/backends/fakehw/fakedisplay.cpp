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

#include "fakedisplay.h"

FakeDisplay::FakeDisplay(FakeDevice *device)
    : FakeCapability(device)
{

}

FakeDisplay::~FakeDisplay()
{

}

Display::DisplayType FakeDisplay::type() const
{

    QString displaytype = fakeDevice()->property("type").toString();

    if ( displaytype=="lcd" )
    {
        return Lcd;
    }
    else if (displaytype=="crt")
    {
        return Crt;
    }
    else if (displaytype=="tv_out")
    {
        return TvOut;
    }
    else
    {
        return UnknownDisplayType;
    }
}

int FakeDisplay::lcdBrightness() const
{
    return fakeDevice()->property("lcdBrightness").toInt();
}

#include "fakedisplay.moc"
