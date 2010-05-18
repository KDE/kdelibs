/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Radek Novacek <rnovacek@redhat.com>
*   Copyright (C) 2009-2010 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "Polkit1Backend.h"

#include <QtCore/qplugin.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

namespace KAuth
{

PolkitResultEventLoop::PolkitResultEventLoop(QObject* parent)
    : QEventLoop(parent)
{
}

PolkitResultEventLoop::~PolkitResultEventLoop()
{
}

void PolkitResultEventLoop::requestQuit(const PolkitQt1::Authority::Result& result)
{
    m_result = result;
    quit();
}

PolkitQt1::Authority::Result PolkitResultEventLoop::result() const
{
    return m_result;
}

Polkit1Backend::Polkit1Backend()
    : AuthBackend()
    , m_flyingActions(false)
{
    setCapabilities(AuthorizeFromHelperCapability | CheckActionExistenceCapability);

    // Setup useful signals
    connect(PolkitQt1::Authority::instance(), SIGNAL(configChanged()),
            this, SLOT(checkForResultChanged()));
    connect(PolkitQt1::Authority::instance(), SIGNAL(consoleKitDBChanged()),
            this, SLOT(checkForResultChanged()));
    connect(PolkitQt1::Authority::instance(), SIGNAL(enumerateActionsFinished(PolkitQt1::ActionDescription::List)),
            this, SLOT(updateCachedActions(PolkitQt1::ActionDescription::List)));

    // Cache existing action IDs as soon as possible
    PolkitQt1::Authority::instance()->enumerateActions();
    m_flyingActions = true;
}

Polkit1Backend::~Polkit1Backend()
{

}

void Polkit1Backend::updateCachedActions(const PolkitQt1::ActionDescription::List& actions)
{
    m_knownActions.clear();
    foreach (PolkitQt1::ActionDescription *action, actions) {
        m_knownActions << action->actionId();
    }
    m_flyingActions = false;
}

Action::AuthStatus Polkit1Backend::authorizeAction(const QString &action)
{
    Q_UNUSED(action)
    // Always return Yes here, we'll authorize inside isCallerAuthorized
    return Action::Authorized;
}

void Polkit1Backend::setupAction(const QString &action)
{
    m_cachedResults[action] = actionStatus(action);
}

Action::AuthStatus Polkit1Backend::actionStatus(const QString &action)
{
    PolkitQt1::UnixProcessSubject subject(QCoreApplication::applicationPid());
    PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync(action, &subject,
                                                                                              PolkitQt1::Authority::None);
    switch (r) {
    case PolkitQt1::Authority::Yes:
        return Action::Authorized;
    case PolkitQt1::Authority::No:
    case PolkitQt1::Authority::Unknown:
        return Action::Denied;
    default:
        return Action::AuthRequired;
    }
}

QByteArray Polkit1Backend::callerID() const
{
    QByteArray a;
    QDataStream s(&a, QIODevice::WriteOnly);
    s << QCoreApplication::applicationPid();

    return a;
}

bool Polkit1Backend::isCallerAuthorized(const QString &action, QByteArray callerID)
{
    QDataStream s(&callerID, QIODevice::ReadOnly);
    qint64 pid;

    s >> pid;

    PolkitQt1::UnixProcessSubject subject(pid);
    PolkitQt1::Authority *authority = PolkitQt1::Authority::instance();

    PolkitResultEventLoop e;
    connect(authority, SIGNAL(checkAuthorizationFinished(PolkitQt1::Authority::Result)),
            &e, SLOT(requestQuit(PolkitQt1::Authority::Result)));
    authority->checkAuthorization(action, &subject, PolkitQt1::Authority::AllowUserInteraction);
    e.exec();

    switch (e.result()) {
    case PolkitQt1::Authority::Yes:
        return true;
    default:
        return false;
    }

    return false;
}

void Polkit1Backend::checkForResultChanged()
{
    foreach(const QString &action, m_cachedResults.keys()) {
        if (m_cachedResults[action] != actionStatus(action)) {
            m_cachedResults[action] = actionStatus(action);
            emit actionStatusChanged(action, m_cachedResults[action]);
        }
    }

    // Force updating known actions
    PolkitQt1::Authority::instance()->enumerateActions();
    m_flyingActions = true;
}

bool Polkit1Backend::actionExists(const QString& action)
{
    // Any flying actions?
    if (m_flyingActions) {
        int tries = 0;
        while (m_flyingActions && tries < 10) {
            // Wait max 2 seconds
            QEventLoop e;
            QTimer::singleShot(200, &e, SLOT(quit()));
            e.exec();
            ++tries;
        }
    }

    return m_knownActions.contains(action);
}

} // namespace Auth

Q_EXPORT_PLUGIN2(kauth_backend, KAuth::Polkit1Backend)
