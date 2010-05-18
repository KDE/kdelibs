/*
*   Copyright (C) 2008 Nicola Gigante <nicola.gigante@gmail.com>
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

#ifndef HELPER_PROXY_H
#define HELPER_PROXY_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

#include "kauthaction.h"
#include "kauthactionreply.h"
#include "kauthactionwatcher.h"

namespace KAuth
{

class HelperProxy : public QObject
{
    Q_OBJECT

public:
    virtual ~HelperProxy();

    // Application-side methods
    virtual bool executeActions(const QList<QPair<QString, QVariantMap> > &list, const QString &helperID) = 0;
    virtual ActionReply executeAction(const QString &action, const QString &helperID, const QVariantMap &arguments) = 0;
    virtual Action::AuthStatus authorizeAction(const QString &action, const QString &helperID) = 0;
    virtual void stopAction(const QString &action, const QString &helperID) = 0;

    // Helper-side methods
    virtual bool initHelper(const QString &name) = 0;
    virtual void setHelperResponder(QObject *o) = 0;
    virtual bool hasToStopAction() = 0;
    virtual void sendDebugMessage(int level, const char *msg) = 0;
    virtual void sendProgressStep(int step) = 0;
    virtual void sendProgressStep(const QVariantMap &step) = 0;

signals:
    void actionStarted(const QString &action);
    void actionPerformed(const QString &action, ActionReply reply);
    void progressStep(const QString &action, int progress);
    void progressStep(const QString &action, const QVariantMap &data);
};

} // namespace KAuth

Q_DECLARE_INTERFACE(KAuth::HelperProxy, "org.kde.auth.HelperProxy/0.1")

#endif
