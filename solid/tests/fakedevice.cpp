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

#include "fakedevice.h"
#include "fakemanager.h"

FakeDevice::FakeDevice(const QString &udi, FakeManager *manager)
    : Device(), m_manager( manager ), m_udi( udi ),
      m_brokenDevice( false ), m_locked( false ),
      m_processor( new FakeProcessor() )
{

}

FakeDevice::~FakeDevice()
{

}

QString FakeDevice::udi() const
{
    return m_udi;
}

QString FakeDevice::parentUdi() const
{
    return m_parent;
}

void FakeDevice::setParent( const QString &udi )
{
    m_parent = udi;
}

QString FakeDevice::vendor() const
{
    return m_data["info.vendor"].toString();
}

QString FakeDevice::product() const
{
    return m_data["info.product"].toString();
}

bool FakeDevice::setProperty( const QString &key, const QVariant &value )
{
    if ( m_brokenDevice ) return false;

    Device::PropertyChange change = Device::PropertyModified;

    if ( !m_data.contains( key ) ) change = Device::PropertyAdded;

    m_data[key] = value;

    emit propertyChanged( key, change );

    return true;
}

QVariant FakeDevice::property( const QString &key ) const
{
    if ( m_brokenDevice || !m_data.contains( key ) ) return QVariant();

    return m_data[key];
}

QMap<QString, QVariant> FakeDevice::allProperties() const
{
    if ( m_brokenDevice ) return QMap<QString, QVariant>();

    return m_data;
}

bool FakeDevice::removeProperty( const QString &key )
{
    if ( m_brokenDevice ) return false;

    m_data.remove( key );
    emit propertyChanged( key, Device::PropertyRemoved );

    return true;
}

bool FakeDevice::propertyExists( const QString &key ) const
{
    if ( m_brokenDevice ) return false;

    return m_data.contains( key );
}

bool FakeDevice::addCapability( const KDEHW::Ifaces::Capability::Type &capability )
{
    if ( m_brokenDevice )
    {
        return false;
    }

    if ( !( m_capabilities & capability ) )
    {
        m_capabilities|= capability;
        emit m_manager->newCapability( m_udi, capability );
    }

    return true;
}

bool FakeDevice::queryCapability( const KDEHW::Ifaces::Capability::Type &capability ) const
{
    if ( m_brokenDevice ) return false;

    return m_capabilities & capability;
}

KDEHW::Ifaces::Capability *FakeDevice::asCapability( const KDEHW::Ifaces::Capability::Type &capability )
{
    if ( ( capability == KDEHW::Ifaces::Capability::Processor )
      && queryCapability( capability ) )
    {
        return m_processor;
    }

    return 0;
}

bool FakeDevice::lock( const QString &reason )
{
    if ( m_brokenDevice || m_locked ) return false;

    m_locked = true;
    m_lockReason = reason;

    return true;
}

bool FakeDevice::unlock()
{
    if ( m_brokenDevice || !m_locked ) return false;

    m_locked = false;
    m_lockReason = QString();

    return true;
}

bool FakeDevice::isLocked() const
{
    return m_locked;
}

QString FakeDevice::lockReason() const
{
    return m_lockReason;
}

void FakeDevice::raiseCondition( const QString &condition, const QString &reason )
{
    emit conditionRaised( condition, reason );
}

bool FakeDevice::isBroken()
{
    return m_brokenDevice;
}

void FakeDevice::setBroken( bool broken )
{
    m_brokenDevice = broken;
}

#include "fakedevice.moc"
