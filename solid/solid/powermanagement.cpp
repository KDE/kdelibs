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

#include "powermanagement.h"
#include "powermanagement_p.h"

#include "soliddefs_p.h"

#include <kglobal.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <klocale.h>

K_GLOBAL_STATIC(Solid::PowerManagementPrivate, globalPowerManager)

Solid::PowerManagementPrivate::PowerManagementPrivate()
    : managerIface("org.kde.Solid.PowerManagement",
                   "/org/kde/Solid/PowerManagement",
                   QDBusConnection::sessionBus()),
      inhibitIface("org.kde.Solid.PowerManagement.Inhibit",
                   "/org/kde/Solid/PowerManagement/Inhibit",
                   QDBusConnection::sessionBus())
{
    powerSaveStatus = managerIface.GetPowerSaveStatus();

    if (managerIface.CanSuspend())
        supportedSleepStates+= Solid::PowerManagement::SuspendState;
    if (managerIface.CanHibernate())
        supportedSleepStates+= Solid::PowerManagement::HibernateState;

    connect(&managerIface, SIGNAL(CanSuspendChanged(bool)),
            this, SLOT(slotCanSuspendChanged(bool)));
    connect(&managerIface, SIGNAL(CanHibernateChanged(bool)),
            this, SLOT(slotCanHibernateChanged(bool)));
    connect(&managerIface, SIGNAL(PowerSaveStatusChanged(bool)),
            this, SLOT(slotPowerSaveStatusChanged(bool)));
}

Solid::PowerManagementPrivate::~PowerManagementPrivate()
{
}

bool Solid::PowerManagement::appShouldConserveResources()
{
    return globalPowerManager->powerSaveStatus;
}

QSet<Solid::PowerManagement::SleepState> Solid::PowerManagement::supportedSleepStates()
{
    return globalPowerManager->supportedSleepStates;
}

QString Solid::PowerManagement::stringForSleepState(SleepState state)
{
    switch (state)
    {
    case StandbyState:
        return i18n("Standby");
    case SuspendState:
        return i18n("Suspend");
    case HibernateState:
        return i18n("Hibernate");
    }
    return QString();
}

void Solid::PowerManagement::requestSleep(SleepState state, QObject *receiver, const char *member)
{
    if (!globalPowerManager->supportedSleepStates.contains(state)) {
        return;
    }

    switch (state)
    {
    case StandbyState:
        break;
    case SuspendState:
        globalPowerManager->managerIface.Suspend();
        break;
    case HibernateState:
        globalPowerManager->managerIface.Hibernate();
        break;
    }
}

int Solid::PowerManagement::beginSuppressingSleep(const QString &reason,
                                                  SuppressExceptions exceptions)
{
    KComponentData componentData = KGlobal::mainComponent();

    QDBusReply<uint> reply = globalPowerManager->inhibitIface.Inhibit(
        componentData.aboutData()->appName(), reason);

    if (reply.isValid())
        return reply;
    else
        return -1;
}

bool Solid::PowerManagement::stopSuppressingSleep(int cookie)
{
    return globalPowerManager->inhibitIface.UnInhibit(cookie).isValid();
}

Solid::PowerManagement::Notifier *Solid::PowerManagement::notifier()
{
    return globalPowerManager;
}

void Solid::PowerManagementPrivate::slotCanSuspendChanged(bool newState)
{
    if (newState) {
        supportedSleepStates+= Solid::PowerManagement::SuspendState;
    } else {
        supportedSleepStates-= Solid::PowerManagement::SuspendState;
    }
}

void Solid::PowerManagementPrivate::slotCanHibernateChanged(bool newState)
{
    if (newState) {
        supportedSleepStates+= Solid::PowerManagement::HibernateState;
    } else {
        supportedSleepStates-= Solid::PowerManagement::HibernateState;
    }
}

void Solid::PowerManagementPrivate::slotPowerSaveStatusChanged(bool newState)
{
    powerSaveStatus = newState;
}

#include "powermanagement_p.moc"
#include "powermanagement.moc"

