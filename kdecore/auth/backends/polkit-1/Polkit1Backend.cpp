/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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
#include <PolkitQt/Authority>
#include <PolkitQt/Subject>
#include <QtCore/QCoreApplication>
//#include <polkit-qt/context.h>
//#include <polkit-qt/auth.h>

namespace KAuth
{

Polkit1Backend::Polkit1Backend()
{
    // Nothing to do here...
}

Action::AuthStatus Polkit1Backend::authorizeAction(const QString &action)
{
    PolkitQt::UnixProcessSubject subject(QCoreApplication::applicationPid());
    switch (PolkitQt::Authority::instance()->checkAuthorizationSync(action, &subject, PolkitQt::Authority::AllowUserInteraction)) {
    case PolkitQt::Authority::Yes:
        return Action::Authorized;
    default:
        return Action::Denied;
    }
}

void Polkit1Backend::setupAction(const QString &action)
{
    connect(PolkitQt::Authority::instance(), SIGNAL(configChanged()),
            this, SLOT(checkForResultChanged()));
    connect(PolkitQt::Authority::instance(), SIGNAL(consoleKitDBChanged()),
            this, SLOT(checkForResultChanged()));

    m_cachedResults[action] = actionStatus(action);
}

Action::AuthStatus Polkit1Backend::actionStatus(const QString &action)
{
    PolkitQt::UnixProcessSubject subject(QCoreApplication::applicationPid());
    PolkitQt::Authority::Result r = PolkitQt::Authority::instance()->checkAuthorizationSync(action, &subject, PolkitQt::Authority::None);
    switch (r) {
    case PolkitQt::Authority::Yes:
        return Action::Authorized;
    case PolkitQt::Authority::No:
    case PolkitQt::Authority::Unknown:
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

    PolkitQt::UnixProcessSubject subject(pid);
    return (PolkitQt::Authority::instance()->checkAuthorizationSync(action, &subject, PolkitQt::Authority::None) == PolkitQt::Authority::Yes);
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
