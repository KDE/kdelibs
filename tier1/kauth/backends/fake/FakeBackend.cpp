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

#include "FakeBackend.h"

namespace KAuth
{

FakeBackend::FakeBackend()
    : AuthBackend()
{
    setCapabilities(NoCapability);
}

Action::AuthStatus FakeBackend::authorizeAction(const QString &action)
{
    Q_UNUSED(action)
    return Action::Denied;
}

void FakeBackend::setupAction(const QString &action)
{
    Q_UNUSED(action)
}

Action::AuthStatus FakeBackend::actionStatus(const QString &action)
{
    Q_UNUSED(action)
    return Action::Denied;
}

QByteArray FakeBackend::callerID() const
{
    return QByteArray();
}

bool FakeBackend::isCallerAuthorized(const QString &action, QByteArray callerID)
{
    Q_UNUSED(action)
    Q_UNUSED(callerID)
    return false;
}

} // namespace Auth
