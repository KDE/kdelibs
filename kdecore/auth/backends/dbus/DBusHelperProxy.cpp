/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009 Dario Freddi <drf@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Lesser General Public License as published by
*   the Free Software Foundation; either version 2.1 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
*/

#include "DBusHelperProxy.h"

#include <QObject>
#include <QMap>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QDebug>
#include <QTimer>

#include <klocalizedstring.h>

#include <syslog.h>

#include "BackendsManager.h"
#include "authadaptor.h"

#ifndef KDE_USE_FINAL
Q_DECLARE_METATYPE(QTimer*)
#endif

namespace KAuth
{

static void debugMessageReceived(int t, const QString &message);

void DBusHelperProxy::stopAction(const QString &action, const QString &helperID)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, "/", "org.kde.auth", "stopAction");

    QList<QVariant> args;
    args << action;
    message.setArguments(args);

    QDBusConnection::systemBus().asyncCall(message);
}

bool DBusHelperProxy::executeActions(const QList<QPair<QString, QVariantMap> > &list, const QString &helperID)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << list;

    QDBusConnection::systemBus().interface()->startService(helperID);

    if (!QDBusConnection::systemBus().connect(helperID, "/", "org.kde.auth", "remoteSignal", this, SLOT(remoteSignalReceived(int, const QString &, QByteArray)))) {
        return false;
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, "/", "org.kde.auth", "performActions");

    QList<QVariant> args;
    args << blob << BackendsManager::authBackend()->callerID();
    message.setArguments(args);

    QDBusPendingCall reply = QDBusConnection::systemBus().asyncCall(message); // This is a NO_REPLY method
    if (reply.reply().type() == QDBusMessage::ErrorMessage) {
        return false;
    }

    return true;
}

ActionReply DBusHelperProxy::executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments)
{
    if (!m_actionsInProgress.isEmpty()) {
        return ActionReply::HelperBusyReply;
    }

    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << arguments;
    
    QDBusConnection::systemBus().interface()->startService(helperID);

    if (!QDBusConnection::systemBus().connect(helperID, "/", "org.kde.auth", "remoteSignal", this, SLOT(remoteSignalReceived(int, const QString &, QByteArray)))) {
        ActionReply errorReply = ActionReply::DBusErrorReply;
        errorReply.setErrorDescription(i18n("DBus Backend error: connection to helper failed. %1", 
                                            QDBusConnection::systemBus().lastError().message()));
        return errorReply;
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, "/", "org.kde.auth", "performAction");

    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID() << blob;
    message.setArguments(args);

    m_actionsInProgress.push_back(action);

    QEventLoop e;
    QDBusPendingCall pendingCall = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher watcher(pendingCall, this);
    connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &e, SLOT(quit()));
    e.exec();
    
    QDBusMessage reply = pendingCall.reply();
    
    if (reply.type() == QDBusMessage::ErrorMessage) {
        ActionReply r = ActionReply::DBusErrorReply;
        r.setErrorDescription(i18n("DBus Backend error: could not contact the helper. "
                                   "Connection error: %1. Message error: %2", QDBusConnection::systemBus().lastError().message(),
                                   reply.errorMessage()));
        qDebug() << reply.errorMessage();

        return r;
    }

    if (reply.arguments().size() != 1) {
        ActionReply errorReply = ActionReply::DBusErrorReply;
        errorReply.setErrorDescription(i18n("DBus Backend error: received corrupt data from helper %1 %2",
                                            reply.arguments().size(), QDBusConnection::systemBus().lastError().message()));
        return errorReply;
    }

    return ActionReply::deserialize(reply.arguments().first().toByteArray());
}

bool DBusHelperProxy::initHelper(const QString &name)
{
    new AuthAdaptor(this);

    if (!QDBusConnection::systemBus().registerService(name)) {
        return false;
    }

    if (!QDBusConnection::systemBus().registerObject("/", this)) {
        return false;
    }

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

    if (type == ActionStarted) {
        emit actionStarted(action);
    } else if (type == ActionPerformed) {
        ActionReply reply;
        stream >> reply;

        m_actionsInProgress.removeOne(action);
        emit actionPerformed(action, reply);
    } else if (type == DebugMessage) {
        int level;
        QString message;

        stream >> level >> message;

        debugMessageReceived(level, message);
    } else if (type == ProgressStepIndicator) {
        int step;
        stream >> step;

        emit progressStep(action, step);
    } else if (type == ProgressStepData) {
        QVariantMap data;
        stream >> data;

        emit progressStep(action, data);
    }
}

void DBusHelperProxy::stopAction(const QString &action)
{
#ifdef __GNUC__
#warning FIXME: The stop request should be action-specific rather than global
#endif
    m_stopRequest = true;
}

bool DBusHelperProxy::hasToStopAction()
{
    QEventLoop loop;
    loop.processEvents(QEventLoop::AllEvents);

    return m_stopRequest;
}

void DBusHelperProxy::performActions(QByteArray blob, const QByteArray &callerID)
{
    QDataStream stream(&blob, QIODevice::ReadOnly);
    QList<QPair<QString, QVariantMap> > actions;

    stream >> actions;

    QList<QPair<QString, QVariantMap> >::const_iterator i = actions.constBegin();
    while (i != actions.constEnd()) {
        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);

        stream << i->second;

        emit remoteSignal(ActionPerformed, i->first, performAction(i->first, callerID, blob));

        i++;
    }
}

QByteArray DBusHelperProxy::performAction(const QString &action, const QByteArray &callerID, QByteArray arguments)
{
    if (!responder) {
        return ActionReply::NoResponderReply.serialized();
    }

    if (!m_currentAction.isEmpty()) {
        return ActionReply::HelperBusyReply.serialized();
    }

    QVariantMap args;
    QDataStream s(&arguments, QIODevice::ReadOnly);
    s >> args;

    if (BackendsManager::authBackend()->isCallerAuthorized(action, callerID)) {
        QTimer *timer = responder->property("__KAuth_Helper_Shutdown_Timer").value<QTimer*>();
        timer->stop();

        QString slotname = action;
        if (slotname.startsWith(m_name + '.')) {
            slotname = slotname.right(slotname.length() - m_name.length() - 1);
        }

        slotname.replace('.', '_');

        ActionReply retVal;

        m_currentAction = action;
        emit remoteSignal(ActionStarted, action, QByteArray());
        bool success = QMetaObject::invokeMethod(responder, slotname.toAscii(), Qt::DirectConnection, Q_RETURN_ARG(ActionReply, retVal), Q_ARG(QVariantMap, args));
        emit remoteSignal(ActionPerformed, action, retVal.serialized());
        m_currentAction = "";

        timer->start();

        if (success) {
            return retVal.serialized();
        } else {
            return ActionReply::NoSuchActionReply.serialized();
        }

    } else {
        return ActionReply::AuthorizationDeniedReply.serialized();
    }
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

void DBusHelperProxy::sendProgressStep(const QVariantMap &data)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << data;

    emit remoteSignal(ProgressStepIndicator, m_currentAction, blob);
}

void debugMessageReceived(int t, const QString &message)
{
    QtMsgType type = (QtMsgType)t;
    switch (type) {
    case QtDebugMsg:
        qDebug("Debug message from helper: %s", message.toAscii().data());
        break;
    case QtWarningMsg:
        qWarning("Warning from helper: %s", message.toAscii().data());
        break;
    case QtCriticalMsg:
        qCritical("Critical warning from helper: %s", message.toAscii().data());
        break;
    case QtFatalMsg:
        qFatal("Fatal error from helper: %s", message.toAscii().data());
        break;
    }
}

} // namespace Auth

#ifndef KDE_USE_FINAL
Q_EXPORT_PLUGIN2(helper_proxy, KAuth::DBusHelperProxy)
#endif
