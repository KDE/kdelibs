/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2010 Dario Freddi <drf@kde.org>
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

#include "AuthBackend.h"

namespace KAuth
{

class AuthBackend::Private
{
public:
    Private() {}
    virtual ~Private() {}

    Capabilities capabilities;
};

AuthBackend::AuthBackend()
    : QObject(0)
    , d(new Private)
{

}

AuthBackend::~AuthBackend()
{
}

AuthBackend::Capabilities AuthBackend::capabilities() const
{
    return d->capabilities;
}

void AuthBackend::setCapabilities(AuthBackend::Capabilities capabilities)
{
    d->capabilities = capabilities;
}

bool AuthBackend::actionExists(const QString& action)
{
    Q_UNUSED(action);
    return false;
}

void AuthBackend::preAuthAction(const QString& action, QWidget* parent)
{
    Q_UNUSED(action)
    Q_UNUSED(parent)
}

} //namespace KAuth

#include "AuthBackend.moc"
