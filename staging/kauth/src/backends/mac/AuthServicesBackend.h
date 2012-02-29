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

#ifndef AUTHSERVICES_BACKEND_H
#define AUTHSERVICES_BACKEND_H

#include "AuthBackend.h"

namespace KAuth
{

class AuthServicesBackend : public AuthBackend
{
    Q_OBJECT
    Q_INTERFACES(KAuth::AuthBackend)

public:
    AuthServicesBackend();
    virtual void setupAction(const QString&);
    virtual Action::AuthStatus authorizeAction(const QString&);
    virtual Action::AuthStatus actionStatus(const QString&);
    virtual QByteArray callerID() const;
    virtual bool isCallerAuthorized(const QString &action, QByteArray callerID);
    virtual bool actionExists(const QString& action);
};

} // namespace KAuth

#endif

