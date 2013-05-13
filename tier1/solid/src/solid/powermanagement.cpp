/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>
    Copyright 2013 Lukas Tinkl <ltinkl@redhat.com>

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

Q_GLOBAL_STATIC(Solid::PowerManagementPrivate, globalPowerManager)

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
                     QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration),
      powerSaveStatus(false)
{
    serviceWatcher.addWatchedService(QLatin1String("org.freedesktop.PowerManagement"));

    connect(&managerIface, SIGNAL(CanSuspendChanged(bool)),
            this, SLOT(slotCanSuspendChanged(bool)));
    connect(&managerIface, SIGNAL(CanHibernateChanged(bool)),
            this, SLOT(slotCanHibernateChanged(bool)));
    connect(&managerIface, SIGNAL(CanHybridSuspendChanged(bool)),
            this, SLOT(slotCanHybridSuspendChanged(bool)));
    connect(&managerIface, SIGNAL(PowerSaveStatusChanged(bool)),
            this, SLOT(slotPowerSaveStatusChanged(bool)));
    connect(&serviceWatcher, SIGNAL(serviceRegistered(QString)),
            this, SLOT(slotServiceRegistered(QString)));
    connect(&serviceWatcher, SIGNAL(serviceUnregistered(QString)),
            this, SLOT(slotServiceUnregistered(QString)));

    // If the service is registered, trigger the connection immediately
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
        slotServiceRegistered("org.kde.Solid.PowerManagement");
    }
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.freedesktop.PowerManagement"))) {
        slotServiceRegistered(QLatin1String("org.freedesktop.PowerManagement"));
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
    return globalPowerManager->managerIface.GetPowerSaveStatus();
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

        if (reply.isValid()) {
            QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.ScreenSaver", "/ScreenSaver",
                                                                  "org.freedesktop.ScreenSaver", "Inhibit");
            message << QCoreApplication::applicationName();
            message << reason;

            QDBusPendingReply<uint> ssReply = QDBusConnection::sessionBus().asyncCall(message);
            ssReply.waitForFinished();
            if (ssReply.isValid()) {
                globalPowerManager->screensaverCookiesForPowerDevilCookies.insert(reply, ssReply.value());
            }

            return reply;
        } else {
            return -1;
        }
    } else {
        // No way to fallback on something, hence return failure
        return -1;
    }
}

bool Solid::PowerManagement::stopSuppressingScreenPowerManagement(int cookie)
{
    if (globalPowerManager->policyAgentIface.isValid()) {
        bool result = globalPowerManager->policyAgentIface.ReleaseInhibition(cookie).isValid();

        if (globalPowerManager->screensaverCookiesForPowerDevilCookies.contains(cookie)) {
            QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.ScreenSaver", "/ScreenSaver",
                                                                  "org.freedesktop.ScreenSaver", "UnInhibit");
            message << globalPowerManager->screensaverCookiesForPowerDevilCookies.take(cookie);
            QDBusConnection::sessionBus().asyncCall(message);
        }

        return result;
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
    if (supportedSleepStates.contains(Solid::PowerManagement::SuspendState) == newState) {
        return;
    }

    if (newState) {
        supportedSleepStates+= Solid::PowerManagement::SuspendState;
    } else {
        supportedSleepStates-= Solid::PowerManagement::SuspendState;
    }
}

void Solid::PowerManagementPrivate::slotCanHibernateChanged(bool newState)
{
    if (supportedSleepStates.contains(Solid::PowerManagement::HibernateState) == newState) {
        return;
    }

    if (newState) {
        supportedSleepStates+= Solid::PowerManagement::HibernateState;
    } else {
        supportedSleepStates-= Solid::PowerManagement::HibernateState;
    }
}

void Solid::PowerManagementPrivate::slotCanHybridSuspendChanged(bool newState)
{
    if (supportedSleepStates.contains(Solid::PowerManagement::HybridSuspendState) == newState) {
        return;
    }

    if (newState) {
        supportedSleepStates+= Solid::PowerManagement::HybridSuspendState;
    } else {
        supportedSleepStates-= Solid::PowerManagement::HybridSuspendState;
    }
}

void Solid::PowerManagementPrivate::slotPowerSaveStatusChanged(bool newState)
{
    if (powerSaveStatus == newState) {
        return;
    }

    powerSaveStatus = newState;
    Q_EMIT appShouldConserveResourcesChanged(powerSaveStatus);
}

void Solid::PowerManagementPrivate::slotServiceRegistered(const QString &serviceName)
{
    if (serviceName == QLatin1String("org.freedesktop.PowerManagement")) {
        // Load all the properties
        QDBusPendingReply<bool> suspendReply = managerIface.CanSuspend();
        suspendReply.waitForFinished();
        slotCanSuspendChanged(suspendReply.isValid() ? suspendReply.value() : false);

        QDBusPendingReply<bool> hibernateReply = managerIface.CanHibernate();
        hibernateReply.waitForFinished();
        slotCanHibernateChanged(hibernateReply.isValid() ? hibernateReply.value() : false);

        QDBusPendingReply<bool> hybridSuspendReply = managerIface.CanHybridSuspend();
        hybridSuspendReply.waitForFinished();
        slotCanHybridSuspendChanged(hybridSuspendReply.isValid() ? hybridSuspendReply.value() : false);

        QDBusPendingReply<bool> saveStatusReply = managerIface.GetPowerSaveStatus();
        saveStatusReply.waitForFinished();
        slotPowerSaveStatusChanged(saveStatusReply.isValid() ? saveStatusReply.value() : false);
    } else {
        // Is the resume signal available?
        QDBusMessage call = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                           "/org/kde/Solid/PowerManagement",
                                                           "org.kde.Solid.PowerManagement",
                                                           "backendCapabilities");
        QDBusPendingReply< uint > reply = QDBusConnection::sessionBus().asyncCall(call);
        reply.waitForFinished();

        if (reply.isValid() && reply.value() > 0) {
                // Connect the signal
                QDBusConnection::sessionBus().connect(QLatin1String("org.kde.Solid.PowerManagement"),
                                                      QLatin1String("/org/kde/Solid/PowerManagement/Actions/SuspendSession"),
                                                      QLatin1String("org.kde.Solid.PowerManagement.Actions.SuspendSession"),
                                                      QLatin1String("resumingFromSuspend"),
                                                      this,
                                                      SIGNAL(resumingFromSuspend()));
        }
    }
}

void Solid::PowerManagementPrivate::slotServiceUnregistered(const QString &serviceName)
{
    if (serviceName == QLatin1String("org.freedesktop.PowerManagement")) {
        // Reset the values
        slotCanSuspendChanged(false);
        slotCanHibernateChanged(false);
        slotCanHybridSuspendChanged(false);
        slotPowerSaveStatusChanged(false);
    } else {
        // Disconnect the signal
        QDBusConnection::sessionBus().disconnect(QLatin1String("org.kde.Solid.PowerManagement"),
                                                 QLatin1String("/org/kde/Solid/PowerManagement"),
                                                 QLatin1String("org.kde.Solid.PowerManagement"),
                                                 QLatin1String("resumingFromSuspend"),
                                                 this,
                                                 SIGNAL(resumingFromSuspend()));
    }
}
