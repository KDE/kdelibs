/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
*   Copyright (C) 2009 Radek Novacek <rnovacek@redhat.com>
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

#include <QDebug>
#include <syslog.h>
#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>
#include <QtCore/QCoreApplication>

namespace KAuth
{

Polkit1Backend::Polkit1Backend()
{
    // Nothing to do here...
}

Action::AuthStatus Polkit1Backend::authorizeAction(const QString &action)
{
    PolkitQt1::UnixProcessSubject subject(QCoreApplication::applicationPid());
    PolkitQt1::Authority *authority = PolkitQt1::Authority::instance();

    switch (authority->checkAuthorizationSync(action, &subject, PolkitQt1::Authority::AllowUserInteraction)) {
    case PolkitQt1::Authority::Yes:
        return Action::Authorized;
    default:
        return Action::Denied;
    }
}

void Polkit1Backend::setupAction(const QString &action)
{
    connect(PolkitQt1::Authority::instance(), SIGNAL(configChanged()),
            this, SLOT(checkForResultChanged()));
    connect(PolkitQt1::Authority::instance(), SIGNAL(consoleKitDBChanged()),
            this, SLOT(checkForResultChanged()));

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
    return (PolkitQt1::Authority::instance()->checkAuthorizationSync(action, &subject, PolkitQt1::Authority::None) ==
            PolkitQt1::Authority::Yes);
}

void Polkit1Backend::checkForResultChanged()
{
    foreach(const QString &action, m_cachedResults.keys()) {
        if (m_cachedResults[action] != actionStatus(action)) {
            m_cachedResults[action] = actionStatus(action);
            emit actionStatusChanged(action, m_cachedResults[action]);
        }
    }
}

} // namespace Auth

Q_EXPORT_PLUGIN2(auth_backend, KAuth::Polkit1Backend)
