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

#ifndef DBUS_HELPER_PROXY_H
#define DBUS_HELPER_PROXY_H

#include <QVariant>
#include "HelperProxy.h"
#include "kauthactionreply.h"

namespace KAuth
{

class DBusHelperProxy : public HelperProxy
{
    Q_OBJECT
    Q_INTERFACES(KAuth::HelperProxy)

    QObject *responder;
    QString m_name;
    QString m_currentAction;
    bool m_stopRequest;
    QList<QString> m_actionsInProgress;

    enum SignalType {
        ActionStarted, // The blob argument is empty
        ActionPerformed, // The blob argument contains the ActionReply
        DebugMessage, // The blob argument contains the debug level and the message (in this order)
        ProgressStepIndicator, // The blob argument contains the step indicator
        ProgressStepData    // The blob argument contains the QVariantMap
    };

public:
    DBusHelperProxy() : responder(0), m_stopRequest(false) {}

    virtual bool executeActions(const QList<QPair<QString, QVariantMap> > &list, const QString &helperID);
    virtual ActionReply executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments);
    virtual Action::AuthStatus authorizeAction(const QString& action, const QString& helperID);
    virtual void stopAction(const QString &action, const QString &helperID);

    virtual bool initHelper(const QString &name);
    virtual void setHelperResponder(QObject *o);
    virtual bool hasToStopAction();
    virtual void sendDebugMessage(int level, const char *msg);
    virtual void sendProgressStep(int step);
    virtual void sendProgressStep(const QVariantMap &data);

public slots:
    void stopAction(const QString &action);
    void performActions(QByteArray blob, const QByteArray &callerID);
    QByteArray performAction(const QString &action, const QByteArray &callerID, QByteArray arguments);
    uint authorizeAction(const QString &action, const QByteArray &callerID);

signals:
    void remoteSignal(int type, const QString &action, const QByteArray &blob); // This signal is sent from the helper to the app

private slots:
    void remoteSignalReceived(int type, const QString &action, QByteArray blob);
};

} // namespace Auth

#endif
