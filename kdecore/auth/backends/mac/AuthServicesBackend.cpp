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

#include "AuthServicesBackend.h"
#include <Security/Security.h>

#include <QtCore/qplugin.h>

namespace KAuth
{

static AuthorizationRef s_authRef = NULL;

AuthorizationRef authRef();

AuthorizationRef authRef()
{
    if (!s_authRef) {
        AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &s_authRef);
    }

    return s_authRef;
}

AuthServicesBackend::AuthServicesBackend()
    : AuthBackend()
{
    setCapabilities(AuthorizeFromHelperCapability | CheckActionExistenceCapability);
}

void AuthServicesBackend::setupAction(const QString&)
{
    // Nothing to do here...
}

// On OS X, the suggestion is to make the helper grant the actual privilege. The app does instead a
// "pre-authorization", that's equivalent to look at isCallerAuthorized() in policykit.
Action::AuthStatus AuthServicesBackend::authorizeAction(const QString &action)
{
    return actionStatus(action);
}

Action::AuthStatus AuthServicesBackend::actionStatus(const QString &action)
{
    AuthorizationItem item;
    item.name = action.toUtf8();
    item.valueLength = 0;
    item.value = NULL;
    item.flags = 0;

    AuthorizationRights rights;
    rights.count = 1;
    rights.items = &item;

    OSStatus result = AuthorizationCopyRights(authRef(),
                      &rights,
                      kAuthorizationEmptyEnvironment,
                      kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize,
                      NULL);

    switch (result) {
    case errAuthorizationSuccess:
        return Action::Authorized;
    case errAuthorizationInteractionNotAllowed:
        return Action::AuthRequired;
    default:
        return Action::Denied;
    }
}

QByteArray AuthServicesBackend::callerID() const
{
    AuthorizationExternalForm ext;
    AuthorizationMakeExternalForm(authRef(), &ext);

    QByteArray id((const char *)&ext, sizeof(ext));

    return id;
}

bool AuthServicesBackend::isCallerAuthorized(const QString &action, QByteArray callerID)
{
    AuthorizationExternalForm ext;
    memcpy(&ext, callerID.data(), sizeof(ext));

    AuthorizationRef auth;

    if (AuthorizationCreateFromExternalForm(&ext, &auth) != noErr)
        return false;

    AuthorizationItem item;
    item.name = action.toUtf8();
    item.valueLength = 0;
    item.value = NULL;
    item.flags = 0;

    AuthorizationRights rights;
    rights.count = 1;
    rights.items = &item;

    OSStatus result = AuthorizationCopyRights(auth,
                      &rights,
                      kAuthorizationEmptyEnvironment,
                      kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed,
                      NULL);

    AuthorizationFree(auth, kAuthorizationFlagDefaults);

    return result == errAuthorizationSuccess;
}

bool AuthServicesBackend::actionExists(const QString& action)
{
    OSStatus exists = AuthorizationRightGet(action.toUtf8(), NULL);

    return exists == errAuthorizationSuccess;
}

}; // namespace KAuth

Q_EXPORT_PLUGIN2(kauth_backend, KAuth::AuthServicesBackend)
