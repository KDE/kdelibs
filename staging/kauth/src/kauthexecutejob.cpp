/*
*   Copyright (C) 2009-2012 Dario Freddi <drf@kde.org>
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

#include "kauthexecutejob.h"
#include "BackendsManager.h"

#include <QEventLoop>
#include <QHash>
#include <QTimer>
#include <QDebug>

namespace KAuth
{

class ExecuteJob::Private
{
public:
    Private(ExecuteJob *parent) : q(parent), isRunning(false), isFinished(false), error(ActionReply::NoError) {}

    ExecuteJob *q;
    Action action;

    Action::ExecutionMode mode;
    bool isRunning;
    bool isFinished;
    bool autoDeleteJob;
    ActionReply::Error error;
    QString errorDescription;
    QVariantMap data;

    void doExecuteAction();
    void doAuthorizeAction();
    void actionPerformedSlot(const QString &action, const ActionReply &reply);
    void progressStepSlot(const QString &action, int i);
    void progressStepSlot(const QString &action, const QVariantMap &data);
    void statusChangedSlot(const QString &action, KAuth::Action::AuthStatus status);
};

static QHash<QString, ExecuteJob *> s_watchers;

ExecuteJob::ExecuteJob(const Action &action, Action::ExecutionMode mode, bool autoDelete, QObject *parent)
        : QObject(parent)
        , d(new Private(this))
{
    d->action = action;
    d->autoDeleteJob = autoDelete;
    d->mode = mode;

    HelperProxy *helper = BackendsManager::helperProxy();

    connect(helper, SIGNAL(actionPerformed(QString,KAuth::ActionReply)), this, SLOT(actionPerformedSlot(QString,KAuth::ActionReply)));
    connect(helper, SIGNAL(progressStep(QString,int)), this, SLOT(progressStepSlot(QString,int)));
    connect(helper, SIGNAL(progressStep(QString,QVariantMap)), this, SLOT(progressStepSlot(QString,QVariantMap)));
    connect(BackendsManager::authBackend(), SIGNAL(actionStatusChanged(QString,KAuth::Action::AuthStatus)),
            this, SLOT(statusChangedSlot(QString,KAuth::Action::AuthStatus)));
}

ExecuteJob::~ExecuteJob()
{
    delete d;
}

Action ExecuteJob::action() const
{
    return d->action;
}

bool ExecuteJob::succeeded() const
{
    return d->error == ActionReply::NoError && d->isFinished;
}

ActionReply::Error ExecuteJob::error() const
{
    return d->error;
}

QString ExecuteJob::errorDescription() const
{
    return d->errorDescription;
}

QVariantMap ExecuteJob::data() const
{
    return d->data;
}

void ExecuteJob::start()
{
    if (!d->action.isValid()) {
        qDebug() << "Tried to start an invalid action";
        ActionReply reply(ActionReply::InvalidActionError);
        reply.setErrorDescription(tr("Tried to start an invalid action"));
        d->actionPerformedSlot(d->action.name(), reply);
        return;
    }

    switch (d->mode) {
    case Action::ExecuteMode:
        QTimer::singleShot(0, this, SLOT(doExecuteAction()));
        break;
    case Action::AuthorizeOnlyMode:
        QTimer::singleShot(0, this, SLOT(doAuthorizeAction()));
        break;
    default:
        {
            ActionReply reply(ActionReply::InvalidActionError);
            reply.setErrorDescription(tr("Unknown execution mode chosen"));
            d->actionPerformedSlot(d->action.name(), reply);
        }
        break;
    }
}

void ExecuteJob::exec()
{
    QEventLoop e;
    connect(this, SIGNAL(finished(KAuth::ExecuteJob*)), &e, SLOT(quit()));
    start();
    e.exec();
}

void ExecuteJob::Private::doExecuteAction()
{
    if (isRunning || isFinished) {
        actionPerformedSlot(action.name(), ActionReply::AlreadyStartedReply());
        return;
    }

    if (!action.isValid()) {
        actionPerformedSlot(action.name(), ActionReply::InvalidActionReply());
        return;
    }

    isRunning = true;

    // If this action authorizes from the client, let's do it now
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
            BackendsManager::authBackend()->preAuthAction(action.name(), action.parentWidget());
        }

        Action::AuthStatus s = BackendsManager::authBackend()->authorizeAction(action.name());

        if (s == Action::StatusAuthorized) {
            if (action.hasHelper()) {
                BackendsManager::helperProxy()->executeAction(action.name(), action.helperID(), action.arguments());
            } else {
                // Done
                actionPerformedSlot(action.name(), ActionReply::SuccessReply());
            }
        } else {
            // Abort if authorization fails
            switch (s) {
            case Action::StatusDenied:
                actionPerformedSlot(action.name(), ActionReply::AuthorizationDeniedReply());
                break;
            case Action::StatusInvalid:
                actionPerformedSlot(action.name(), ActionReply::InvalidActionReply());
                break;
            case Action::StatusUserCancelled:
                actionPerformedSlot(action.name(), ActionReply::UserCancelledReply());
                break;
            default:
                {
                    ActionReply r(ActionReply::BackendError);
                    r.setErrorDescription(tr("Unknown status for the authentication procedure"));
                    actionPerformedSlot(action.name(), r);
                }
                break;
            }
        }
    } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
        if (!action.hasHelper()) {
            ActionReply r(ActionReply::InvalidActionReply());
            r.setErrorDescription(tr("The current backend only allows helper authorization, but this action does not have an helper."));
            actionPerformedSlot(action.name(), r);
            return;
        }
        BackendsManager::helperProxy()->executeAction(action.name(), action.helperID(), action.arguments());
    } else {
        // There's something totally wrong here
        ActionReply r(ActionReply::BackendError);
        r.setErrorDescription(tr("The backend does not specify how to authorize"));
        actionPerformedSlot(action.name(), r);
    }
}

void ExecuteJob::Private::doAuthorizeAction()
{
    // Check the status first
    Action::AuthStatus s = action.status();
    if (s == Action::StatusAuthRequired) {
        // Let's check what to do
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
            // In this case we can actually try an authorization
            if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
                BackendsManager::authBackend()->preAuthAction(action.name(), action.parentWidget());
            }

            s = BackendsManager::authBackend()->authorizeAction(action.name());
        } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
            // In this case, just throw out success, as the auth will take place later
            s = Action::StatusAuthorized;
        } else {
            // This should never, never happen
            ActionReply r(ActionReply::BackendError);
            r.setErrorDescription(tr("The backend does not specify how to authorize"));
            actionPerformedSlot(action.name(), r);
        }
    }

    // Return based on the current status
    if (s == Action::StatusAuthorized) {
        actionPerformedSlot(action.name(), ActionReply::SuccessReply());
    } else {
        actionPerformedSlot(action.name(), ActionReply::AuthorizationDeniedReply());
    }
}

void ExecuteJob::Private::actionPerformedSlot(const QString &taction, const ActionReply &reply)
{
    if (taction == action.name()) {
        if (reply.failed()) {
            error = reply.errorCode();
            errorDescription = reply.errorDescription();
        } else {
            data = reply.data();
        }

        isRunning = false;
        isFinished = true;

        Q_EMIT q->finished(q);

        if (autoDeleteJob) {
            q->deleteLater();
        }
    }
}

void ExecuteJob::Private::progressStepSlot(const QString &taction, int i)
{
    if (taction == action.name()) {
        Q_EMIT q->percent(i);
    }
}

void ExecuteJob::Private::progressStepSlot(const QString &taction, const QVariantMap &data)
{
    if (taction == action.name()) {
        Q_EMIT q->newData(data);
    }
}

void ExecuteJob::Private::statusChangedSlot(const QString &taction, Action::AuthStatus status)
{
    if (taction == action.name()) {
        Q_EMIT q->statusChanged(status);
    }
}

} // namespace Auth

#include "moc_kauthexecutejob.cpp"
