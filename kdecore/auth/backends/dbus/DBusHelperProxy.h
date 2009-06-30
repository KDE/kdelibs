#ifndef DBUS_HELPER_BACKEND_H
#define DBUS_HELPER_BACKEND_H

#include "HelperProxy.h"
#include "ActionReply.h"

class DBusHelperProxy : public QObject, public HelperProxy
{
    Q_OBJECT
    Q_INTERFACES(HelperProxy)
    
    QObject *responder;
    
    public:
        DBusHelperProxy() : responder(NULL) {}
        
        virtual ActionReply executeAction(const QString &action, const QMap<QString, QVariant> &arguments);
        
        virtual bool initHelper(const QString &name);
        
        virtual void setHelperResponder(QObject *o);
        
    public slots:
        virtual bool performAction(const QString &action, QByteArray callerID, QByteArray arguments); // this is private
};

#endif