/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

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

#include "device.h"

KDEHW::Ifaces::Device::Device( QObject *parent )
    : QObject( parent )
{

}

KDEHW::Ifaces::Device::~Device()
{

}

QString KDEHW::Ifaces::Device::parentUdi() const
{
    return QString();
}

bool KDEHW::Ifaces::Device::setProperty( const QString &/*key*/, const QVariant &/*value*/ )
{
    return false;
}

bool KDEHW::Ifaces::Device::removeProperty( const QString &/*key*/ )
{
    return false;
}

bool KDEHW::Ifaces::Device::addCapability( const Capability::Type &/*capability*/ )
{
    return false;
}

bool KDEHW::Ifaces::Device::lock(const QString &/*reason*/)
{
    return false;
}

bool KDEHW::Ifaces::Device::unlock()
{
    return false;
}

bool KDEHW::Ifaces::Device::isLocked() const
{
    return false;
}

QString KDEHW::Ifaces::Device::lockReason() const
{
    return QString();
}

#include "device.moc"
