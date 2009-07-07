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
#include "authadaptor.h"

static void debugMessageReceived(int t, QString message);

void DBusHelperProxy::stopAction(const QString &action, const QString &helperID)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, "/", "org.kde.auth", "stopAction");
    
    QList<QVariant> args;
    args << action;
    message.setArguments(args);
    
    QDBusConnection::systemBus().call(message, QDBus::NoBlock);
}

bool DBusHelperProxy::executeActions(const QList<QPair<QString, QVariantMap> > &list, const QString &helperID)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    
    stream << list;
    
    if(!QDBusConnection::systemBus().connect(helperID, "/", "org.kde.auth", "remoteSignal", this, SLOT(remoteSignalReceived(int, const QString &, QByteArray))))
        return false;
    
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, "/", "org.kde.auth", "performActions");
    
    QList<QVariant> args;
    args << blob << BackendsManager::authBackend()->callerID();
    message.setArguments(args);
    
    QDBusMessage reply = QDBusConnection::systemBus().call(message, QDBus::NoBlock); // This is a NO_REPLY method
    if(reply.type() == QDBusMessage::ErrorMessage)
        return false;
    
    return true;
}

ActionReply DBusHelperProxy::executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments)
{
    if(!m_actionsInProgress.isEmpty())
        return ActionReply::HelperBusyReply;
    
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    
    stream << arguments;
    
    if(!QDBusConnection::systemBus().connect(helperID, "/", "org.kde.auth", "remoteSignal", this, SLOT(remoteSignalReceived(int, const QString &, QByteArray))))
        return false;
    
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, "/", "org.kde.auth", "performAction");
    
    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID() << blob;
    message.setArguments(args);
    
    m_actionsInProgress.push_back(action);
    
    QDBusMessage reply = QDBusConnection::systemBus().call(message, QDBus::BlockWithGui); 
    if(reply.type() == QDBusMessage::ErrorMessage)
    {
        ActionReply r = ActionReply::DBusErrorReply;
        r.setErrorDescription(reply.errorMessage());
        
        return r;
    }
    
    if(reply.arguments().size() != 1)
        return ActionReply::WrongReplyDataReply;
    
    return reply.arguments().first().toByteArray();
}

bool DBusHelperProxy::initHelper(const QString &name)
{
    new AuthAdaptor(this);
    
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

void DBusHelperProxy::remoteSignalReceived(int t, const QString &action, QByteArray blob)
{
    SignalType type = (SignalType)t;
    QDataStream stream(&blob, QIODevice::ReadOnly);
    
    if(type == ActionStarted)
    {
        ActionWatcher::watcher(action)->emitActionStarted();
    }else if(type == ActionPerformed)
    {
        ActionReply reply;
        stream >> reply;
        
        m_actionsInProgress.removeOne(action);
        ActionWatcher::watcher(action)->emitActionPerformed(reply);
    }else if(type == DebugMessage)
    {
        int level;
        QString message;
        
        stream >> level >> message;
        
        debugMessageReceived(level, message);
    }else if(type == ProgressStepIndicator)
    {
        int step;
        stream >> step;
        
        ActionWatcher::watcher(action)->emitProgressStep(step);
    }else if(type == ProgressStepData)
    {
        QVariantMap data;
        stream >> data;
        
        ActionWatcher::watcher(action)->emitProgressStep(data);
    }
}

void DBusHelperProxy::stopAction(QString action)
{
    m_stopRequest = true;
}

bool DBusHelperProxy::hasToStopAction()
{
    QEventLoop loop;
    loop.processEvents(QEventLoop::AllEvents);
    
    return m_stopRequest;
}

void DBusHelperProxy::performActions(QByteArray blob, QByteArray callerID)
{    
    QDataStream stream(&blob, QIODevice::ReadOnly);
    QList<QPair<QString, QVariantMap> > actions;
    
    stream >> actions;
    
    QList<QPair<QString, QVariantMap> >::const_iterator i = actions.constBegin();
    while (i != actions.constEnd())
    {
        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        
        stream << i->second;
        
        emit remoteSignal(ActionPerformed, i->first, performAction(i->first, callerID, blob));
        
        i++;
    }
}

QByteArray DBusHelperProxy::performAction(const QString &action, QByteArray callerID, QByteArray arguments)
{
    if(!responder)
        return ActionReply::NoResponderReply.serialized();
    
    if(!m_currentAction.isEmpty())
        return ActionReply::HelperBusyReply.serialized();
    
    QVariantMap args;
    QDataStream s(&arguments, QIODevice::ReadOnly);
    s >> args;
    
    if(BackendsManager::authBackend()->isCallerAuthorized(action, callerID))
    {
        QString slotname = action;
        if(slotname.startsWith(m_name + "."))
            slotname = slotname.right(slotname.length() - m_name.length() - 1);
        
        slotname.replace(".", "_");
        
        ActionReply retVal;
        
        m_currentAction = action;
        emit remoteSignal(ActionStarted, action, QByteArray());
        bool success = QMetaObject::invokeMethod(responder, slotname.toAscii(), Qt::DirectConnection, Q_RETURN_ARG(ActionReply, retVal), Q_ARG(QVariantMap, args));
        m_currentAction = "";
        
        if(success)
            return retVal.serialized();
        else
            return ActionReply::NoSuchActionReply.serialized();
        
    }else
        return ActionReply::AuthorizationDeniedReply.serialized();
}

void DBusHelperProxy::sendDebugMessage(int level, const char *msg)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    
    stream << level << QString(msg);
    
    emit remoteSignal(DebugMessage, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStep(int step)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    
    stream << step;
    
    emit remoteSignal(ProgressStepIndicator, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStep(QVariantMap data)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);
    
    stream << data;
    
    emit remoteSignal(ProgressStepIndicator, m_currentAction, blob);
}

void debugMessageReceived(int t, QString message)
{
    QtMsgType type = (QtMsgType)t;
    switch(type)
    {
        case QtDebugMsg:
            qDebug("Message from helper: %s", message.toAscii().data());
            break;
        case QtWarningMsg:
            qWarning("Message from helper: %s", message.toAscii().data());
            break;
        case QtCriticalMsg:
            qCritical("Message from helper: %s", message.toAscii().data());
            break;
        case QtFatalMsg:
            qFatal("Message from helper: %s", message.toAscii().data());
            break;
    }
}

Q_EXPORT_PLUGIN2(helper_proxy, DBusHelperProxy);