/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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

#include "TestBackend.h"

#include <QCoreApplication>
#include <QDebug>

namespace KAuth
{

TestBackend::TestBackend()
    : AuthBackend()
{
    qDebug() << "Test backend loaded";
    setCapabilities(AuthorizeFromHelperCapability | CheckActionExistenceCapability);
}

void TestBackend::setNewCapabilities(AuthBackend::Capabilities capabilities)
{
    qDebug() << "Capabilities changing";
    setCapabilities(capabilities);
}

Action::AuthStatus TestBackend::authorizeAction(const QString &action)
{
    if (action == QLatin1String("doomed.to.fail")) {
        return Action::StatusDenied;
    }

    return Action::StatusAuthorized;
}

void TestBackend::setupAction(const QString &action)
{
    if (action == QLatin1String("doomed.to.fail")) {
        m_actionStatuses.insert(action, Action::StatusDenied);
    } else if (action == QLatin1String("requires.auth") || action == QLatin1String("generates.error")) {
        m_actionStatuses.insert(action, Action::StatusAuthRequired);
    } else if (action == QLatin1String("always.authorized")) {
        m_actionStatuses.insert(action, Action::StatusAuthorized);
    } else if (action.startsWith(QLatin1String("org.kde.auth.autotest"))) {
        m_actionStatuses.insert(action, Action::StatusAuthRequired);
    }
}

Action::AuthStatus TestBackend::actionStatus(const QString &action)
{
    if (m_actionStatuses.contains(action)) {
        return m_actionStatuses.value(action);
    }

    return Action::StatusInvalid;
}

QByteArray TestBackend::callerID() const
{
    return QByteArray("a random caller Id");
}

bool TestBackend::isCallerAuthorized(const QString &action, QByteArray callerId)
{
    if (action == QLatin1String("doomed.to.fail")) {
        return false;
    } else if (action == QLatin1String("requires.auth")) {
        m_actionStatuses.insert(action, Action::StatusAuthorized);
        Q_EMIT actionStatusChanged(action, Action::StatusAuthorized);
        return true;
    } else if (action == QLatin1String("generates.error")) {
        m_actionStatuses.insert(action, Action::StatusError);
        Q_EMIT actionStatusChanged(action, Action::StatusError);
        return false;
    } else if (action == QLatin1String("always.authorized")) {
        return true;
    } else if (action.startsWith(QLatin1String("org.kde.auth.autotest"))) {
        qDebug() << "Caller ID:" << callerId;
        if (callerId == callerID()) {
            m_actionStatuses.insert(action, Action::StatusAuthorized);
            Q_EMIT actionStatusChanged(action, Action::StatusAuthorized);
            return true;
        } else {
            m_actionStatuses.insert(action, Action::StatusDenied);
            Q_EMIT actionStatusChanged(action, Action::StatusDenied);
        }
    }

    return false;
}

bool TestBackend::actionExists(const QString& action)
{
    qDebug() << "Checking if action " << action << "exists";
    if (action != QLatin1String("doomed.to.fail") &&
        action != QLatin1String("requires.auth") &&
        action != QLatin1String("generates.error") &&
        action != QLatin1String("always.authorized") &&
        action != QLatin1String("/safinvalid124%$&") &&
        !action.startsWith(QLatin1String("org.kde.auth.autotest"))) {
        return false;
    }

    return true;
}

} // namespace Auth
