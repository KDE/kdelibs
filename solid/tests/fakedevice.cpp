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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <kdebug.h>

#include "fakedevice.h"


FakeDevice::FakeDevice(const QString &udi)
    : Device()
{

}

FakeDevice::~FakeDevice()
{

}

QString FakeDevice::udi() const
{
    return property( "info.udi" ).toString();
}

bool FakeDevice::setProperty( const QString &key, const QVariant &value )
{
    return false;
}

QVariant FakeDevice::property( const QString &key ) const
{
    return QVariant();
}

QMap<QString, QVariant> FakeDevice::allProperties() const
{
    return QMap<QString, QVariant>();
}

bool FakeDevice::removeProperty( const QString &key )
{
    return false;
}

bool FakeDevice::propertyExists( const QString &key ) const
{
    return false;
}

bool FakeDevice::addCapability( const QString &capability )
{
    return false;
}

bool FakeDevice::queryCapability( const QString &capability ) const
{
    return false;
}

bool FakeDevice::lock(const QString &reason)
{
    return false;
}

bool FakeDevice::unlock()
{
    return false;
}


#include "fakedevice.moc"
