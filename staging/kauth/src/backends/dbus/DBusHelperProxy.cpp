/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
*   Copyright (C) 2009-2010 Dario Freddi <drf@kde.org>
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

#include <QtCore/qplugin.h>
#include <QObject>
#include <QMap>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QDebug>
#include <QTimer>

#include <syslog.h>

#include "BackendsManager.h"
#include "authadaptor.h"

Q_DECLARE_METATYPE(QTimer*)

namespace KAuth
{

static void debugMessageReceived(int t, const QString &message);

void DBusHelperProxy::stopAction(const QString &action, const QString &helperID)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.auth"), QLatin1String("stopAction"));

    QList<QVariant> args;
    args << action;
    message.setArguments(args);

    QDBusConnection::systemBus().asyncCall(message);
}

void DBusHelperProxy::executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << arguments;

    QDBusConnection::systemBus().interface()->startService(helperID);

    if (!QDBusConnection::systemBus().connect(helperID, QLatin1String("/"), QLatin1String("org.kde.auth"), QLatin1String("remoteSignal"), this, SLOT(remoteSignalReceived(int,QString,QByteArray)))) {
        ActionReply errorReply = ActionReply::DBusErrorReply;
        errorReply.setErrorDescription(tr("DBus Backend error: connection to helper failed. ")
				       + QDBusConnection::systemBus().lastError().message());
        Q_EMIT actionPerformed(action, errorReply);
    }

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.auth"), QLatin1String("performAction"));

    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID() << blob;
    message.setArguments(args);

    m_actionsInProgress.push_back(action);

    QDBusPendingCall pendingCall = QDBusConnection::systemBus().asyncCall(message);

    if (pendingCall.reply().type() == QDBusMessage::ErrorMessage) {
        ActionReply r = ActionReply::DBusErrorReply;
        r.setErrorDescription(tr("DBus Backend error: could not contact the helper. "
                "Connection error: ") + QDBusConnection::systemBus().lastError().message() + tr(". Message error: ") + pendingCall.reply().errorMessage());
        qDebug() << pendingCall.reply().errorMessage();

        Q_EMIT actionPerformed(action, r);
    }
}

Action::AuthStatus DBusHelperProxy::authorizeAction(const QString& action, const QString& helperID)
{
    if (!m_actionsInProgress.isEmpty()) {
        return Action::StatusError;
    }

    QDBusConnection::systemBus().interface()->startService(helperID);

    QDBusMessage message;
    message = QDBusMessage::createMethodCall(helperID, QLatin1String("/"), QLatin1String("org.kde.auth"), QLatin1String("authorizeAction"));

    QList<QVariant> args;
    args << action << BackendsManager::authBackend()->callerID();
    message.setArguments(args);

    m_actionsInProgress.push_back(action);

    QEventLoop e;
    QDBusPendingCall pendingCall = QDBusConnection::systemBus().asyncCall(message);
    QDBusPendingCallWatcher watcher(pendingCall, this);
    connect(&watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), &e, SLOT(quit()));
    e.exec();

    m_actionsInProgress.removeOne(action);

    QDBusMessage reply = pendingCall.reply();

    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().size() != 1) {
        return Action::StatusError;
    }

    return static_cast<Action::AuthStatus>(reply.arguments().first().toUInt());
}

bool DBusHelperProxy::initHelper(const QString &name)
{
    new AuthAdaptor(this);

    if (!QDBusConnection::systemBus().registerService(name)) {
        return false;
    }

    if (!QDBusConnection::systemBus().registerObject(QLatin1String("/"), this)) {
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
        Q_EMIT actionStarted(action);
    } else if (type == ActionPerformed) {
        ActionReply reply = ActionReply::deserialize(blob);

        m_actionsInProgress.removeOne(action);
        Q_EMIT actionPerformed(action, reply);
    } else if (type == DebugMessage) {
        int level;
        QString message;

        stream >> level >> message;

        debugMessageReceived(level, message);
    } else if (type == ProgressStepIndicator) {
        int step;
        stream >> step;

        Q_EMIT progressStep(action, step);
    } else if (type == ProgressStepData) {
        QVariantMap data;
        stream >> data;

        Q_EMIT progressStep(action, data);
    }
}

void DBusHelperProxy::stopAction(const QString &action)
{
    Q_UNUSED(action)
//#warning FIXME: The stop request should be action-specific rather than global
    m_stopRequest = true;
}

bool DBusHelperProxy::hasToStopAction()
{
    QEventLoop loop;
    loop.processEvents(QEventLoop::AllEvents);

    return m_stopRequest;
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

    m_currentAction = action;
    Q_EMIT remoteSignal(ActionStarted, action, QByteArray());
    QEventLoop e;
    e.processEvents(QEventLoop::AllEvents);

    ActionReply retVal;

    QTimer *timer = responder->property("__KAuth_Helper_Shutdown_Timer").value<QTimer*>();
    timer->stop();

    if (BackendsManager::authBackend()->isCallerAuthorized(action, callerID)) {
        QString slotname = action;
        if (slotname.startsWith(m_name + QLatin1Char('.'))) {
            slotname = slotname.right(slotname.length() - m_name.length() - 1);
        }

        slotname.replace(QLatin1Char('.'), QLatin1Char('_'));

        bool success = QMetaObject::invokeMethod(responder, slotname.toAscii().data(), Qt::DirectConnection,
                                                 Q_RETURN_ARG(ActionReply, retVal), Q_ARG(QVariantMap, args));

        if (!success) {
            retVal = ActionReply::NoSuchActionReply;
        }

    } else {
        retVal = ActionReply::AuthorizationDeniedReply;
    }

    timer->start();

    Q_EMIT remoteSignal(ActionPerformed, action, retVal.serialized());
    e.processEvents(QEventLoop::AllEvents);
    m_currentAction.clear();
    m_stopRequest = false;

    return retVal.serialized();
}


uint DBusHelperProxy::authorizeAction(const QString& action, const QByteArray& callerID)
{
    if (!m_currentAction.isEmpty()) {
        return static_cast<uint>(Action::StatusError);
    }

    m_currentAction = action;

    uint retVal;

    QTimer *timer = responder->property("__KAuth_Helper_Shutdown_Timer").value<QTimer*>();
    timer->stop();

    if (BackendsManager::authBackend()->isCallerAuthorized(action, callerID)) {
        retVal = static_cast<uint>(Action::StatusAuthorized);
    } else {
        retVal = static_cast<uint>(Action::StatusDenied);
    }

    timer->start();
    m_currentAction.clear();

    return retVal;
}


void DBusHelperProxy::sendDebugMessage(int level, const char *msg)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << level << QString::fromLocal8Bit(msg);

    Q_EMIT remoteSignal(DebugMessage, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStep(int step)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << step;

    Q_EMIT remoteSignal(ProgressStepIndicator, m_currentAction, blob);
}

void DBusHelperProxy::sendProgressStep(const QVariantMap &data)
{
    QByteArray blob;
    QDataStream stream(&blob, QIODevice::WriteOnly);

    stream << data;

    Q_EMIT remoteSignal(ProgressStepData, m_currentAction, blob);
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

Q_EXPORT_PLUGIN2(kauth_helper_backend, KAuth::DBusHelperProxy)
