#ifndef ACTION_H
#define ACTION_H

#include <QString>
#include <QVariant>
#include <QHash>

#include "ActionReply.h"

class Action
{
    QString m_name;
    QMap<QString, QVariant> m_args;
    
    public:
        enum AuthStatus
        {
            Denied = 0,
            Authorized = 1,
            AuthRequired = 2
        };
        
        Action(const char *name) : m_name(name) { init(); }
        Action(const QString &name) : m_name(name) { init(); }
        
        QString name() { return m_name; }
        void setName(QString name) { m_name = name; }
        
        QMap<QString, QVariant> &arguments() { return m_args; }
        
        bool authorize();
        AuthStatus status();
        ActionReply execute();
        
    private:
        void init();
};

#endif
