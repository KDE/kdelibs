#include <QtDebug>
#include <auth.h>
#include "Backend.h"

namespace PolicyKit
{

    Backend::Backend()
    {
        // Nothing to do here...
    }
        
    bool Backend::authorizeAction(const QString &action)
    {
        return PolkitQt::Auth::computeAndObtainAuth(action);
    }

    void Backend::setupAction(const QString &action)
    {
        // Nothing to do here...
    }

    Action::AuthStatus Backend::actionStatus(const QString &action)
    {
        PolkitQt::Auth::Result r = PolkitQt::Auth::isCallerAuthorized(action, QCoreApplication::applicationPid(), false);
        switch(r)
        {
            case PolkitQt::Auth::Yes:
                return Action::Authorized;
            case PolkitQt::Auth::No:
            case PolkitQt::Auth::Unknown:
                return Action::Denied;
            default:
                return Action::AuthRequired;
        }
    }
    
    QByteArray Backend::callerID()
    {
        QByteArray a;
        QDataStream s(&a, QIODevice::WriteOnly);
        s << QCoreApplication::applicationPid();
        
        return a;
    }
    
    bool Backend::isCallerAuthorized(const QString &action, QByteArray callerID)
    {
        QDataStream s(&callerID, QIODevice::ReadOnly);
        qint64 pid;
        
        s >> pid;
        
        return (PolkitQt::Auth::isCallerAuthorized(action, pid, true) == PolkitQt::Auth::Yes);
    }
}

Q_EXPORT_PLUGIN2(auth_backend, PolicyKit::Backend);