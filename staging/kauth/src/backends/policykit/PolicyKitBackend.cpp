/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
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

#include "PolicyKitBackend.h"

#include <QtCore/qplugin.h>
#include <syslog.h>
#include <polkit-qt/context.h>
#include <polkit-qt/auth.h>

namespace KAuth
{

PolicyKitBackend::PolicyKitBackend()
    : AuthBackend()
{
    setCapabilities(AuthorizeFromClientCapability);
}

Action::AuthStatus PolicyKitBackend::authorizeAction(const QString &action)
{
    switch (PolkitQt::Auth::computeAndObtainAuth(action)) {
    case PolkitQt::Auth::Yes:
        return Action::Authorized;
    default:
        return Action::Denied;
    }
}

void PolicyKitBackend::setupAction(const QString &action)
{
    connect(PolkitQt::Context::instance(), SIGNAL(configChanged()),
            this, SLOT(checkForResultChanged()));
    connect(PolkitQt::Context::instance(), SIGNAL(consoleKitDBChanged()),
            this, SLOT(checkForResultChanged()));

    m_cachedResults[action] = actionStatus(action);
}

Action::AuthStatus PolicyKitBackend::actionStatus(const QString &action)
{
    PolkitQt::Auth::Result r = PolkitQt::Auth::isCallerAuthorized(action, QCoreApplication::applicationPid(),
                               false);
    switch (r) {
    case PolkitQt::Auth::Yes:
        return Action::Authorized;
    case PolkitQt::Auth::No:
    case PolkitQt::Auth::Unknown:
        return Action::Denied;
    default:
        return Action::AuthRequired;
    }
}

QByteArray PolicyKitBackend::callerID() const
{
    QByteArray a;
    QDataStream s(&a, QIODevice::WriteOnly);
    s << QCoreApplication::applicationPid();

    return a;
}

bool PolicyKitBackend::isCallerAuthorized(const QString &action, QByteArray callerID)
{
    QDataStream s(&callerID, QIODevice::ReadOnly);
    qint64 pid;

    s >> pid;

    return (PolkitQt::Auth::isCallerAuthorized(action, pid, false) == PolkitQt::Auth::Yes);
}

void PolicyKitBackend::checkForResultChanged()
{
    QHash<QString, Action::AuthStatus>::iterator i;
    for (i = m_cachedResults.begin(); i != m_cachedResults.end(); ++i) {
        if (i.value() != actionStatus(i.key())) {
            i.value() = actionStatus(i.key());
            emit actionStatusChanged(i.key(), i.value());
        }
    }
}

} // namespace Auth

Q_EXPORT_PLUGIN2(kauth_backend, KAuth::PolicyKitBackend)
