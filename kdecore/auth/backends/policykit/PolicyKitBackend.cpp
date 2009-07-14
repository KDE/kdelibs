/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "PolicyKitBackend.h"

#include <QDebug>
#include <auth.h>
#include <syslog.h>

PolicyKitBackend::PolicyKitBackend()
{
    // Nothing to do here...
}

Action::AuthStatus PolicyKitBackend::authorizeAction(const QString &action)
{    
    return PolkitQt::Auth::computeAndObtainAuth(action) ? Action::Authorized : Action::Denied;
}

void PolicyKitBackend::setupAction(const QString &action)
{
    // Nothing to do here...
}

Action::AuthStatus PolicyKitBackend::actionStatus(const QString &action)
{
    PolkitQt::Auth::Result r = PolkitQt::Auth::isCallerAuthorized(action, QCoreApplication::applicationPid(), false);
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

    return (PolkitQt::Auth::isCallerAuthorized(action, pid, true) == PolkitQt::Auth::Yes);
}

Q_EXPORT_PLUGIN2(auth_backend, PolicyKitBackend);
