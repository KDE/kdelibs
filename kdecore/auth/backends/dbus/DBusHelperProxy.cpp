/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*                                                               
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or   
*   (at your option) any later version.                                 
*                                                                       
*   This program is distributed in the hope that it will be useful,     
*   but WITHOUT ANY WARRANTY; without even the implied warranty of      
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       
*   GNU General Public License for more details.                        
*                                                                       
*   You should have received a copy of the GNU General Public License   
*   along with this program; if not, write to the                       
*   Free Software Foundation, Inc.,                                     
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .      
*/

#include <QMap>
#include <QtDBus>
#include <QDebug>

#include "BackendsManager.h"
#include "DBusHelperProxy.h"
#include "DBusHelperProxyAdaptor.h"

ActionReply DBusHelperProxy::executeAction(const QString &action, const QMap<QString, QVariant> &arguments)
{
    QByteArray argsBytes;
    QDataStream stream(&argsBytes, QIODevice::WriteOnly);
    
    stream << arguments;
        
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("it.gigabytes.auth",
                                              "/",
                                              "it.gigabytes.auth",
                                              QLatin1String("performAction"));
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(action) << BackendsManager::authBackend()->callerID() << qVariantFromValue(argsBytes);
    message.setArguments(argumentList);
    
    QDBusMessage reply = QDBusConnection::systemBus().call(message);
//     if (reply.type() == QDBusMessage::ReplyMessage
//         && reply.arguments().size() == 1) {
//         // the reply can be anything, here we receive a string
//         qDebug() << reply.arguments().first().toString();
//     } else if (reply.type() == QDBusMessage::MethodCallMessage) {
//         qWarning() << "Message did not receive a reply (timeout by message bus)";
//     }

    return ActionReply();
}

bool DBusHelperProxy::initHelper(const QString &name)
{
    new DBusHelperProxyAdaptor(this);
    
    if (!QDBusConnection::systemBus().registerService(name))
        return false;
    
    if (!QDBusConnection::systemBus().registerObject("/", this))
        return false;
    
    return true;
}

void DBusHelperProxy::setHelperResponder(QObject *o)
{
    responder = o;
}

bool DBusHelperProxy::performAction(const QString &action, QByteArray callerID, QByteArray arguments)
{
    if(!responder)
        return false;
    
    QMap<QString, QVariant> args;
    QDataStream s(&arguments, QIODevice::ReadOnly);
    s >> args;
    
    // TODO: Check authorization (using callerID)
    if(BackendsManager::authBackend()->isCallerAuthorized(action, callerID))
        QMetaObject::invokeMethod(responder, "action", Q_ARG(ArgumentsMap, args));
}

Q_EXPORT_PLUGIN2(helper_proxy, DBusHelperProxy);