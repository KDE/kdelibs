/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "powermanager.h"

#include "soliddefs_p.h"
#include "managerbase_p.h"
#include "ifaces/powermanager.h"

namespace Solid
{
    class PowerManagerPrivate : public ManagerBasePrivate
    {
        Q_DECLARE_PUBLIC(PowerManager)
    public:
        void connectBackend( QObject *newBackend );
    };
}

SOLID_SINGLETON_IMPLEMENTATION( Solid::PowerManager, PowerManager )

Solid::PowerManager::PowerManager()
    : ManagerBase(*new PowerManagerPrivate, "Power Management",
                  "SolidPowerManager", "Solid::Ifaces::PowerManager" )
{
    Q_D(PowerManager);

    if ( managerBackend() != 0 )
    {
        d->connectBackend( managerBackend() );
    }
}

Solid::PowerManager::~PowerManager()
{

}

QStringList Solid::PowerManager::supportedSchemes() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), QStringList(), supportedSchemes() );
}

QString Solid::PowerManager::schemeDescription( const QString &schemeName ) const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), QString(), schemeDescription( schemeName ) );
}

QString Solid::PowerManager::scheme() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), QString(), scheme() );
}

bool Solid::PowerManager::setScheme( const QString &name )
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), false, setScheme( name ) );
}

Solid::PowerManager::BatteryState Solid::PowerManager::batteryState() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), NoBatteryState, batteryState() );
}

int Solid::PowerManager::batteryChargePercent() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), -1, batteryChargePercent() );
}

Solid::PowerManager::AcAdapterState Solid::PowerManager::acAdapterState() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), Plugged, acAdapterState() );
}

Solid::PowerManager::SuspendMethods Solid::PowerManager::supportedSuspendMethods() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), UnknownSuspendMethod, supportedSuspendMethods() );
}

KJob *Solid::PowerManager::suspend( SuspendMethod method ) const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), 0, suspend( method ) );
}

Solid::PowerManager::CpuFreqPolicies Solid::PowerManager::supportedCpuFreqPolicies() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), UnknownCpuFreqPolicy, supportedCpuFreqPolicies() );
}

Solid::PowerManager::CpuFreqPolicy Solid::PowerManager::cpuFreqPolicy() const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), UnknownCpuFreqPolicy, cpuFreqPolicy() );
}

bool Solid::PowerManager::setCpuFreqPolicy( CpuFreqPolicy newPolicy )
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), false, setCpuFreqPolicy( newPolicy ) );
}

bool Solid::PowerManager::canDisableCpu( int cpuNum ) const
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), false, canDisableCpu( cpuNum ) );
}

bool Solid::PowerManager::setCpuEnabled( int cpuNum, bool enabled )
{
    return_SOLID_CALL( Ifaces::PowerManager*, managerBackend(), false, setCpuEnabled( cpuNum, enabled ) );
}

void Solid::PowerManager::setManagerBackend( QObject *backend )
{
    Q_D(PowerManager);

    ManagerBase::setManagerBackend(backend);
    if (backend) {
        d->connectBackend(backend);
    }
}

void Solid::PowerManagerPrivate::connectBackend( QObject *newBackend )
{
    Q_Q(PowerManager);

    QObject::connect( newBackend, SIGNAL( schemeChanged( QString ) ),
                      q, SIGNAL( schemeChanged( QString ) ) );
    QObject::connect( newBackend, SIGNAL( acAdapterStateChanged( int ) ),
                      q, SIGNAL( acAdapterStateChanged( int ) ) );
    QObject::connect( newBackend, SIGNAL( batteryStateChanged( int ) ),
                      q, SIGNAL( batteryStateChanged( int ) ) );
    QObject::connect( newBackend, SIGNAL( buttonPressed( int ) ),
                      q, SIGNAL( buttonPressed( int ) ) );
}

#include "powermanager.moc"

