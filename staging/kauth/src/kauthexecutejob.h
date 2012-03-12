/*
*   Copyright (C) 2012 Dario Freddi <drf@kde.org>
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

#ifndef ASYNC_ACTION_H
#define ASYNC_ACTION_H

#include <QtCore/QObject>

#include <kauth_export.h>

#include "kauthaction.h"
#include "kauthactionreply.h"

namespace KAuth
{

class KAUTH_EXPORT ExecuteJob : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ExecuteJob)

    ExecuteJob(const KAuth::Action &action, KAuth::Action::ExecutionMode mode, bool autoDelete, QObject *parent);

    friend class Action;

    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void doExecuteAction())
    Q_PRIVATE_SLOT(d, void doAuthorizeAction())
    Q_PRIVATE_SLOT(d, void actionPerformedSlot(const QString &action, const KAuth::ActionReply &reply))
    Q_PRIVATE_SLOT(d, void progressStepSlot(const QString &action, int i))
    Q_PRIVATE_SLOT(d, void progressStepSlot(const QString &action, const QVariantMap &data))
    Q_PRIVATE_SLOT(d, void statusChangedSlot(const QString &action, KAuth::Action::AuthStatus status))

public:
    /// Virtual destructor
    virtual ~ExecuteJob();

    /// Returns the action associated with this job
    Action action() const;

    void start();

    bool succeeded() const;

    ActionReply::Error error() const;
    QString errorDescription() const;

    QVariantMap data() const;

Q_SIGNALS:
    /**
    * @brief Signal emitted by the helper to notify the action's progress
    *
    * This signal is emitted every time the helper's code calls the
    * HelperSupport::progressStep(QVariantMap) method. This is useful to let the
    * helper notify the execution status of a long action, also providing
    * some data, for example if you want to achieve some sort of progressive loading.
    * The meaning of the data passed here is totally application-dependent.
    * If you only need to pass some percentage, you can use the other signal that
    * pass an int.
    *
    * @param data The progress data from the helper
    */
    void newData(const QVariantMap &data);

    void percent(uint percent);

    void finished(ExecuteJob *job);

    void statusChanged(KAuth::Action::AuthStatus status);
};

} // namespace Auth

#endif
