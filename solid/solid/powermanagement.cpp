/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
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
      policyAgentIface("org.kde.Solid.PowerManagement.PolicyAgent",
                       "/org/kde/Solid/PowerManagement/PolicyAgent",
                       QDBusConnection::sessionBus()),
      inhibitIface("org.freedesktop.PowerManagement.Inhibit",
                   "/org/freedesktop/PowerManagement/Inhibit",
                   QDBusConnection::sessionBus()),
      serviceWatcher("org.kde.Solid.PowerManagement",
                     QDBusConnection::sessionBus(),
                     QDBusServiceWatcher::WatchForRegistration)
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
    connect(&serviceWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(slotServiceRegistered(QString)));

    // If the service is registered, trigger the connection immediately
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
        slotServiceRegistered("org.kde.Solid.PowerManagement");
    }
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
    QDBusReply<uint> reply;
    if (globalPowerManager->policyAgentIface.isValid()) {
        reply = globalPowerManager->policyAgentIface.AddInhibition(
            (uint)PowerManagementPrivate::InterruptSession,
            QCoreApplication::applicationName(), reason);
    } else {
        // Fallback to the fd.o Inhibit interface
        reply = globalPowerManager->inhibitIface.Inhibit(QCoreApplication::applicationName(), reason);
    }

    if (reply.isValid())
        return reply;
    else
        return -1;
}

bool Solid::PowerManagement::stopSuppressingSleep(int cookie)
{
    if (globalPowerManager->policyAgentIface.isValid()) {
        return globalPowerManager->policyAgentIface.ReleaseInhibition(cookie).isValid();
    } else {
        // Fallback to the fd.o Inhibit interface
        return globalPowerManager->inhibitIface.UnInhibit(cookie).isValid();
    }
}

int Solid::PowerManagement::beginSuppressingScreenPowerManagement(const QString& reason)
{
    if (globalPowerManager->policyAgentIface.isValid()) {
        QDBusReply<uint> reply = globalPowerManager->policyAgentIface.AddInhibition(
            (uint)PowerManagementPrivate::ChangeScreenSettings,
            QCoreApplication::applicationName(), reason);

        if (reply.isValid())
            return reply;
        else
            return -1;
    } else {
        // No way to fallback on something, hence return failure
        return -1;
    }
}

bool Solid::PowerManagement::stopSuppressingScreenPowerManagement(int cookie)
{
    if (globalPowerManager->policyAgentIface.isValid()) {
        return globalPowerManager->policyAgentIface.ReleaseInhibition(cookie).isValid();
    } else {
        // No way to fallback on something, hence return failure
        return false;
    }
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

void Solid::PowerManagementPrivate::slotServiceRegistered(const QString &serviceName)
{
    Q_UNUSED(serviceName);

    // Is the resume signal available?
    QDBusMessage call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                       "/org/kde/Solid/PowerManagement",
                                                       "org.kde.Solid.PowerManagement",
                                                       "backendCapabilities");
    QDBusPendingReply< uint > reply = QDBusConnection::sessionBus().asyncCall(call);
    reply.waitForFinished();

    if (reply.isValid() && reply.value() > 0) {
        // Connect the signal
        QDBusConnection::sessionBus().connect("org.kde.Solid.PowerManagement",
                                              "/org/kde/Solid/PowerManagement",
                                              "org.kde.Solid.PowerManagement",
                                              "resumingFromSuspend",
                                              this,
                                              SIGNAL(resumingFromSuspend()));
    }
}

#include "powermanagement_p.moc"
#include "powermanagement.moc"

