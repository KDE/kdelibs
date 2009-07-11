/*
 *  AuthServicesBackend.cpp
 *  
 *
 *  Created by Nicola Gigante on 09/07/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "AuthServicesBackend.h"
#include <Security/Security.h>

static AuthorizationRef s_authRef = NULL;

AuthorizationRef authRef();

AuthorizationRef authRef()
{
	if(!s_authRef)
		AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &s_authRef);
	
	return s_authRef;
}

AuthServicesBackend::AuthServicesBackend() : QObject(NULL) {}

void AuthServicesBackend::setupAction(const QString&)
{
	// Nothing to do here...
}

// On OS X, the suggestion is to make the helper grant the actual privilege. The app does instead a 
// "pre-authorization", that's equivalent to look at isCallerAuthorized() in policykit.
bool AuthServicesBackend::authorizeAction(const QString &action)
{	
	return actionStatus(action) == Action::Authorized;
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
	
	switch(result)
	{
		case errAuthorizationSuccess:
			return Action::Authorized;
		case errAuthorizationInteractionNotAllowed:
			return Action::AuthRequired;
		default:
			return Action::Denied;
	}
}

QByteArray AuthServicesBackend::callerID()
{
	AuthorizationExternalForm ext;
	AuthorizationMakeExternalForm(authRef(), &ext);
	
	QByteArray id((const char *)&ext, sizeof(ext));
	
	return id;
}

bool AuthServicesBackend::isCallerAuthorized(const QString &action, QByteArray callerID)
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
											  kAuthorizationFlagExtendRights | kAuthorizationFlagInteractionAllowed,
											  NULL);
	
	switch(result)
	{
		case errAuthorizationSuccess:
			return Action::Authorized;
		case errAuthorizationInteractionNotAllowed:
			return Action::AuthRequired;
		default:
			return Action::Denied;
	}
}

Q_EXPORT_PLUGIN2(auth_backend, AuthServicesBackend);
