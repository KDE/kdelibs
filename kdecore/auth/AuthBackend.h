/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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

#ifndef AUTH_BACKEND_H
#define AUTH_BACKEND_H

#include <QObject>

#include "kauthaction.h"

namespace KAuth
{

class AuthBackend : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AuthBackend)

public:
    enum Capability {
        NoCapability = 0,
        AuthorizeFromClientCapability = 1,
        AuthorizeFromHelperCapability = 2,
        CheckActionExistenceCapability = 4
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    AuthBackend();
    virtual ~AuthBackend();
    virtual void setupAction(const QString &action) = 0;
    virtual Action::AuthStatus authorizeAction(const QString &action) = 0;
    virtual Action::AuthStatus actionStatus(const QString &action) = 0;
    virtual QByteArray callerID() const = 0;
    virtual bool isCallerAuthorized(const QString &action, QByteArray callerID) = 0;
    virtual bool actionExists(const QString &action);

    Capabilities capabilities() const;

protected:
    void setCapabilities(Capabilities capabilities);

Q_SIGNALS:
    void actionStatusChanged(const QString &action, Action::AuthStatus status);

private:
    class Private;
    Private * const d;
};

} // namespace Auth

Q_DECLARE_INTERFACE(KAuth::AuthBackend, "org.kde.auth.AuthBackend/0.1")
Q_DECLARE_OPERATORS_FOR_FLAGS(KAuth::AuthBackend::Capabilities)

#endif
