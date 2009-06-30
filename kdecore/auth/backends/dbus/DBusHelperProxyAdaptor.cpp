#include "DBusHelperProxyAdaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QString>


/*
 * Implementation of adaptor class QDBusHelperProxyAdaptor
 */

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

void DBusHelperProxyAdaptor::performAction(const QString &action, const QByteArray &callerID, const QByteArray &arguments)
{
    // handle method call org.kde.auth.performAction
    QMetaObject::invokeMethod(parent(), "performAction", Q_ARG(QString, action), Q_ARG(QByteArray, callerID), Q_ARG(QByteArray, arguments));
}

