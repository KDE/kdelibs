/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "halblock.h"

#include "haldevice.h"

using namespace Solid::Backends::Hal;

Block::Block(HalDevice *device)
    : DeviceInterface(device)
{

}

Block::~Block()
{

}

int Block::deviceMajor() const
{
    return m_device->property("block.major").toInt();
}

int Block::deviceMinor() const
{
    return m_device->property("block.minor").toInt();
}

QString Block::device() const
{
    return m_device->property("block.device").toString();
}

#include "backends/hal/halblock.moc"
