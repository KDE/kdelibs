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

#include <QtCore/QCoreApplication>

SOLID_GLOBAL_STATIC(Solid::PowerManagementPrivate, globalPowerManager)

Solid::PowerManagementPrivate::PowerManagementPrivate()
    : managerIface("org.freedesktop.PowerManagement",
                   "/org/freedesktop/PowerManagement",
                   QDBusConnection::sessionBus()),
      inhibitIface("org.freedesktop.PowerManagement.Inhibit",
                   "/org/freedesktop/PowerManagement/Inhibit",
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

Solid::PowerManagement::Notifier::Notifier()
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

void Solid::PowerManagement::requestSleep(SleepState state, QObject *receiver, const char *member)
{
    Q_UNUSED(receiver)
    Q_UNUSED(member)

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

int Solid::PowerManagement::beginSuppressingSleep(const QString &reason)
{
    QDBusReply<uint> reply = globalPowerManager->inhibitIface.Inhibit(
        QCoreApplication::applicationName(), reason);

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
    emit appShouldConserveResourcesChanged(powerSaveStatus);
}

#include "powermanagement_p.moc"
#include "powermanagement.moc"

