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
#include "powermanager_p.h"

#include "../soliddefs_p.h"
#include "managerbase_p.h"
#include "ifaces/powermanager.h"

#include <kglobal.h>

K_GLOBAL_STATIC(SolidExperimental::PowerManagerPrivate, globalPowerManager)

SolidExperimental::PowerManagerPrivate::PowerManagerPrivate()
{
    loadBackend("Power Management", "SolidPowerManager", "SolidExperimental::Ifaces::PowerManager");

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(schemeChanged(QString)),
                this, SIGNAL(schemeChanged(QString)));
        connect(managerBackend(), SIGNAL(acAdapterStateChanged(int)),
                this, SIGNAL(acAdapterStateChanged(int)));
        connect(managerBackend(), SIGNAL(batteryStateChanged(int)),
                this, SIGNAL(batteryStateChanged(int)));
        connect(managerBackend(), SIGNAL(buttonPressed(int)),
                this, SIGNAL(buttonPressed(int)));
    }
}

SolidExperimental::PowerManagerPrivate::~PowerManagerPrivate()
{
}

QStringList SolidExperimental::PowerManager::supportedSchemes()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      QStringList(), supportedSchemes());
}

QString SolidExperimental::PowerManager::schemeDescription(const QString &schemeName)
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      QString(), schemeDescription(schemeName));
}

QString SolidExperimental::PowerManager::scheme()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(), QString(), scheme());
}

bool SolidExperimental::PowerManager::setScheme(const QString &name)
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      false, setScheme(name));
}

SolidExperimental::PowerManager::BatteryState SolidExperimental::PowerManager::batteryState()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      NoBatteryState, batteryState());
}

int SolidExperimental::PowerManager::batteryChargePercent()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      -1, batteryChargePercent());
}

SolidExperimental::PowerManager::AcAdapterState SolidExperimental::PowerManager::acAdapterState()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      Plugged, acAdapterState());
}

SolidExperimental::PowerManager::SuspendMethods SolidExperimental::PowerManager::supportedSuspendMethods()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      UnknownSuspendMethod, supportedSuspendMethods());
}

KJob *SolidExperimental::PowerManager::suspend(SuspendMethod method)
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      0, suspend(method));
}

SolidExperimental::PowerManager::CpuFreqPolicies SolidExperimental::PowerManager::supportedCpuFreqPolicies()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      UnknownCpuFreqPolicy, supportedCpuFreqPolicies());
}

SolidExperimental::PowerManager::CpuFreqPolicy SolidExperimental::PowerManager::cpuFreqPolicy()
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      UnknownCpuFreqPolicy, cpuFreqPolicy());
}

bool SolidExperimental::PowerManager::setCpuFreqPolicy(CpuFreqPolicy newPolicy)
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      false, setCpuFreqPolicy(newPolicy));
}

bool SolidExperimental::PowerManager::canDisableCpu(int cpuNum)
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      false, canDisableCpu(cpuNum));
}

bool SolidExperimental::PowerManager::setCpuEnabled(int cpuNum, bool enabled)
{
    return_SOLID_CALL(Ifaces::PowerManager*, globalPowerManager->managerBackend(),
                      false, setCpuEnabled(cpuNum, enabled));
}

SolidExperimental::PowerManager::Notifier *SolidExperimental::PowerManager::notifier()
{
    return globalPowerManager;
}

#include "powermanager_p.moc"
#include "powermanager.moc"

