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

#ifndef POLKIT1BACKEND_H
#define POLKIT1BACKEND_H

#include "AuthBackend.h"
#include <QHash>

class QByteArray;

namespace KAuth
{

class Polkit1Backend : public AuthBackend
{
    Q_OBJECT
    Q_INTERFACES(KAuth::AuthBackend)

public:
    Polkit1Backend();
    virtual void setupAction(const QString&);
    virtual Action::AuthStatus authorizeAction(const QString&);
    virtual Action::AuthStatus actionStatus(const QString&);
    virtual QByteArray callerID() const;
    virtual bool isCallerAuthorized(const QString &action, QByteArray callerID);

Q_SIGNALS:
    void actionStatusChanged(const QString &action, Action::AuthStatus status);

private Q_SLOTS:
    void checkForResultChanged();

private:
    QHash<QString, Action::AuthStatus> m_cachedResults;
};

} // namespace Auth

#endif
