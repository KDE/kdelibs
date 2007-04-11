/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "brightnesscontrol.h"

namespace Phonon
{
namespace Fake
{
BrightnessControl::BrightnessControl(QObject *parent)
    : VideoEffect(-1, parent)
    , m_brightness(0)
{
}

BrightnessControl::~BrightnessControl()
{
}

int BrightnessControl::brightness() const
{
    return m_brightness;
}

void BrightnessControl::setBrightness(int brightness)
{
    m_brightness = brightness;
}

int BrightnessControl::lowerBound() const
{
    return -1000;
}

int BrightnessControl::upperBound() const
{
    return 1000;
}

}} //namespace Phonon::Fake

#include "brightnesscontrol.moc"
// vim: sw=4 ts=4
