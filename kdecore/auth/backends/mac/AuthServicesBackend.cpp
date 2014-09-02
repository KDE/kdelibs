/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2014 René Bertin <rjvbertin@gmail.com>
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
#include <QtCore/QtCore>

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

static OSStatus GetActionRights(const QString &action, AuthorizationFlags flags, AuthorizationRef auth=NULL)
{
    AuthorizationItem item;
    item.name = action.toUtf8();
    item.valueLength = 0;
    item.value = NULL;
    item.flags = 0;

    AuthorizationRights rights;
    rights.count = 1;
    rights.items = &item;

    OSStatus result = AuthorizationCopyRights( (auth)? auth : authRef(),
                                              &rights,
                                              kAuthorizationEmptyEnvironment,
                                              flags, NULL);
    return result;
}

// On OS X, the suggestion is to make the helper grant the actual privilege. The app does instead a
// "pre-authorization", that's equivalent to look at isCallerAuthorized() in policykit.
// RJVB: grab the privilege from here, the client.
AuthServicesBackend::AuthServicesBackend()
    : AuthBackend()
{
    setCapabilities(AuthorizeFromClientCapability | CheckActionExistenceCapability);
}

void AuthServicesBackend::setupAction(const QString&)
{
    // Nothing to do here...
}

// On OS X, the suggestion is to make the helper grant the actual privilege. The app does instead a
// "pre-authorization", that's equivalent to look at isCallerAuthorized() in policykit.
// RJVB: grab the privilege from here, the client.
Action::AuthStatus AuthServicesBackend::authorizeAction(const QString &action)
{
    OSStatus result = GetActionRights( action, kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed );
//    qWarning() << "AuthServicesBackend::authorizeAction(" << action << ") AuthorizationCopyRights returned" << result;
    switch (result) {
        case errAuthorizationSuccess:
            return Action::Authorized;
        case errAuthorizationInteractionNotAllowed:
        default:
            return Action::Denied;
    }
}

Action::AuthStatus AuthServicesBackend::actionStatus(const QString &action)
{
    OSStatus result = GetActionRights( action, kAuthorizationFlagExtendRights | kAuthorizationFlagPreAuthorize );
//    qWarning() << "AuthServicesBackend::actionStatus(" << action << ") AuthorizationCopyRights returned" << result;
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

    if (AuthorizationCreateFromExternalForm(&ext, &auth) != noErr){
//        qWarning() << "AuthorizationCreateFromExternalForm(" << action << "," << callerID.constData() << ") failed";
        return false;
    }

    OSStatus result = GetActionRights( action, kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed,
                      auth);

    AuthorizationFree(auth, kAuthorizationFlagDefaults);
//    qWarning() << "AuthServicesBackend::isCallerAuthorized(" << action << "," << callerID.constData() << ") AuthorizationCopyRights returned" << result;

    return result == errAuthorizationSuccess;
}

// RJVB: OS X doesn't distinguish between "action doesn't exist" and "action not allowed". So the
// best thing we can do is return true and hope that the action will be created if it didn't exist...
bool AuthServicesBackend::actionExists(const QString& action)
{
    OSStatus exists = AuthorizationRightGet(action.toUtf8(), NULL);
//    qWarning() << "AuthServicesBackend::actionExists(" << action << ") AuthorizationRightGet returned" << exists;

    return true;//exists == errAuthorizationSuccess;
}

}; // namespace KAuth

Q_EXPORT_PLUGIN2(kauth_backend, KAuth::AuthServicesBackend)
