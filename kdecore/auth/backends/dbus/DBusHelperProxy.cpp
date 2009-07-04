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

#include <QObject>
#include <QMap>
#include <QtDBus>
#include <QDebug>

#include <syslog.h>

#include "BackendsManager.h"
#include "DBusHelperProxy.h"
#include "DBusHelperProxyAdaptor.h"

ActionReply DBusHelperProxy::executeAction(const QString &action, const QString &helperID, const ArgumentsMap &arguments)
{
    QByteArray argsBytes;
    QDataStream stream(&argsBytes, QIODevice::WriteOnly);
    
    stream << arguments;
        
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, // We use the helper ID as the service name
                                              "/",
                                              "org.kde.auth",
                                              QLatin1String("performAction"));
                                              
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue(action) << BackendsManager::authBackend()->callerID() << qVariantFromValue(argsBytes);
    message.setArguments(argumentList);
    
    // TODO: Check for dbus errors
    QDBusMessage reply = QDBusConnection::systemBus().call(message);
    
    if(reply.type() == QDBusMessage::ErrorMessage)
    {
        ActionReply r = ActionReply::DBusErrorReply;
        r.setErrorDescription(reply.errorMessage());
        
        return r;
    }
    
    if(reply.arguments().size() != 1)
        return ActionReply::WrongReplyDataReply;
    
    return ActionReply(reply.arguments().first().toByteArray());
}

bool DBusHelperProxy::initHelper(const QString &name)
{
    new DBusHelperProxyAdaptor(this);
    
    if (!QDBusConnection::systemBus().registerService(name))
        return false;
    
    if (!QDBusConnection::systemBus().registerObject("/", this))
        return false;
    
    m_name = name;
    
    return true;
}

void DBusHelperProxy::setHelperResponder(QObject *o)
{
    responder = o;
}

void DBusHelperProxy::performActionAsync(const QString &action, QByteArray callerID, QByteArray arguments)
{
    emit actionPerformed(performAction(action, callerID, arguments));
}

QByteArray DBusHelperProxy::performAction(const QString &action, QByteArray callerID, QByteArray arguments)
{
    if(!responder)
        return ActionReply::NoResponderReply.serialized();
    
    ArgumentsMap args;
    QDataStream s(&arguments, QIODevice::ReadOnly);
    s >> args;
    
    if(BackendsManager::authBackend()->isCallerAuthorized(action, callerID))
    {
        QString slotname = action;
        if(slotname.startsWith(m_name + "."))
            slotname = slotname.right(slotname.length() - m_name.length() - 1);
        
        slotname.replace(".", "_");
        
        ActionReply retVal;
        bool success = QMetaObject::invokeMethod(responder, slotname.toAscii(), Qt::DirectConnection, Q_RETURN_ARG(ActionReply, retVal), Q_ARG(ArgumentsMap, args));
        
        if(success)
            return retVal.serialized();
        else
            return ActionReply::NoSuchActionReply.serialized();
        
    }else
        return ActionReply::AuthorizationDeniedReply.serialized();
}

Q_EXPORT_PLUGIN2(helper_proxy, DBusHelperProxy);