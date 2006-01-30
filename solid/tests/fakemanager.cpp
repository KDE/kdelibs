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

#include <kdebug.h>

#include "fakemanager.h"
#include "fakedevice.h"


FakeManager::FakeManager()
    : DeviceManager()
{
}

FakeManager::~FakeManager()
{

}

QStringList FakeManager::allDevices()
{
    return m_devices.keys();
}

bool FakeManager::deviceExists( const QString &udi )
{
    return m_devices.contains( udi );
}

KDEHW::Ifaces::Device *FakeManager::createDevice( const QString &udi )
{
    if ( m_devices.contains( udi ) )
    {
        return m_devices[udi];
    }
    else
    {
        return 0;
    }
}

QStringList FakeManager::findDeviceStringMatch( const QString &key, const QString &value )
{
    QStringList matches;

    foreach ( QString udi, m_devices.keys() )
    {
        FakeDevice *dev = m_devices[udi];

        if ( dev->propertyExists( key )
          && dev->property( key ) == value )
        {
            matches.append( udi );
        }
    }

    return matches;
}

QStringList FakeManager::findDeviceByCapability( const KDEHW::Ifaces::Capability::Type &capability )
{
    QStringList matches;

    foreach ( QString udi, m_devices.keys() )
    {
        FakeDevice *dev = m_devices[udi];

        if ( dev->queryCapability( capability ) )
        {
            matches.append( udi );
        }
    }

    return matches;
}

FakeDevice *FakeManager::newDevice( const QString &udi )
{
    if ( m_devices.contains( udi ) )
    {
        kdDebug() << "Trying to create a new device, but it already exists" << endl;
        return 0;
    }
    else
    {
        FakeDevice *dev = new FakeDevice( udi, this );
        m_devices[udi] = dev;
        emit deviceAdded( udi );
        return dev;
    }
}

FakeDevice *FakeManager::findDevice( const QString &udi )
{
    if ( m_devices.contains( udi ) )
    {
        return m_devices[udi];
    }
    else
    {
        return 0;
    }
}

void FakeManager::deleteDevice( const QString &udi )
{
    if ( m_devices.contains( udi ) )
    {
        emit deviceRemoved( udi );

        m_devices.remove( udi );
    }
}

#include "fakemanager.moc"
