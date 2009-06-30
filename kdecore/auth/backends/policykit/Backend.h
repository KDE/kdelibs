#ifndef POLICYKIT_BACKEND_H
#define POLICYKIT_BACKEND_H

#include "AuthBackend.h"

class QByteArray;

namespace PolicyKit
{

class Backend : public QObject, public AuthBackend
{
    Q_OBJECT
    Q_INTERFACES(AuthBackend)
    
    public:
        Backend();
        virtual void setupAction(const QString&);
        virtual bool authorizeAction(const QString&);
        virtual Action::AuthStatus actionStatus(const QString&);
        virtual QByteArray callerID();
        virtual bool isCallerAuthorized(const QString &action, QByteArray callerID);
};

}

#endif