#ifndef AUTH_BACKEND_H
#define AUTH_BACKEND_H

#include <QtPlugin>

#include "Action.h"

class AuthBackend
{
    public:
        virtual void setupAction(const QString &action) = 0;
        virtual bool authorizeAction(const QString &action) = 0;
        virtual Action::AuthStatus actionStatus(const QString &action) = 0;
        virtual QByteArray callerID() = 0;
        virtual bool isCallerAuthorized(const QString &action, QByteArray callerID) = 0;
};

Q_DECLARE_INTERFACE(AuthBackend, "org.kde.auth.AuthBackend/0.1");

#endif