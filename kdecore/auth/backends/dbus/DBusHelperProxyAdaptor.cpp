#include "DBusHelperProxyAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QString>


DBusHelperProxyAdaptor::DBusHelperProxyAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

DBusHelperProxyAdaptor::~DBusHelperProxyAdaptor()
{
    // destructor
}

void DBusHelperProxyAdaptor::performActionAsync(const QString &action, const QByteArray &callerID, const QByteArray &arguments)
{
    // handle method call org.kde.auth.performActionAsync
    QMetaObject::invokeMethod(parent(), "performActionAsync", Q_ARG(QString, action), Q_ARG(QByteArray, callerID), Q_ARG(QByteArray, arguments));
}

QByteArray DBusHelperProxyAdaptor::performAction(const QString &action, const QByteArray &callerID, const QByteArray &arguments)
{
    // handle method call org.kde.auth.performActionAsync
    QByteArray ret;
    QMetaObject::invokeMethod(parent(), "performAction", Q_RETURN_ARG(QByteArray, ret), Q_ARG(QString, action), Q_ARG(QByteArray, callerID), Q_ARG(QByteArray, arguments));
    
    return ret;
}