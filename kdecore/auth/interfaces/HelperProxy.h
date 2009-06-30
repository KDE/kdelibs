#ifndef HELPER_BACKEND
#define HELPER_BACKEND

#include <QtPlugin>

#include "ActionReply.h"

template<class Key, class T> class QMap;
class QString;
class QVariant;

class HelperProxy
{
    public:
        typedef QMap<QString, QVariant> ArgumentsMap;
        
        ActionReply executeAction(const QString &action) { return executeAction(action, ArgumentsMap()); }
        
        virtual ActionReply executeAction(const QString &action, const ArgumentsMap &arguments) = 0;
        virtual bool initHelper(const QString &name) = 0;
        virtual void setHelperResponder(QObject *o) = 0;
};

Q_DECLARE_INTERFACE(HelperProxy, "org.kde.auth.HelperProxy/0.1");

#endif