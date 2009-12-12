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

#include "kauthactionwatcher.h"
#include "BackendsManager.h"

#include <QHash>

namespace KAuth
{

class ActionWatcher::Private
{
public:
    Private(ActionWatcher *parent) : q(parent) {}

    ActionWatcher *q;
    QString action;

    void actionStartedSlot(const QString &action);
    void actionPerformedSlot(const QString &action, const ActionReply &reply);
    void progressStepSlot(const QString &action, int i);
    void progressStepSlot(const QString &action, const QVariantMap &data);
    void statusChangedSlot(const QString &action, Action::AuthStatus status);
};

static QHash<QString, ActionWatcher *> s_watchers;

ActionWatcher::ActionWatcher(const QString &action)
        : QObject(0)
        , d(new Private(this))
{
    d->action = action;

    HelperProxy *helper = BackendsManager::helperProxy();

    connect(helper, SIGNAL(actionStarted(QString)), this, SLOT(actionStartedSlot(QString)));
    connect(helper, SIGNAL(actionPerformed(QString, ActionReply)), this, SLOT(actionPerformedSlot(QString, ActionReply)));
    connect(helper, SIGNAL(progressStep(QString, int)), this, SLOT(progressStepSlot(QString, int)));
    connect(helper, SIGNAL(progressStep(QString, QVariantMap)), this, SLOT(progressStepSlot(QString, QVariantMap)));
    connect(BackendsManager::authBackend(), SIGNAL(actionStatusChanged(QString, Action::AuthStatus)),
            this, SLOT(statusChangedSlot(QString, Action::AuthStatus)));
}

ActionWatcher::~ActionWatcher()
{
    delete d;
}

ActionWatcher *ActionWatcher::watcher(const QString &action)
{
    if (!s_watchers.contains(action)) {
        s_watchers[action] = new ActionWatcher(action);
    }

    return s_watchers[action];
}

QString ActionWatcher::action() const
{
    return d->action;
}

void ActionWatcher::Private::actionStartedSlot(const QString &taction)
{
    if (taction == action) {
        emit q->actionStarted();
    }
}

void ActionWatcher::Private::actionPerformedSlot(const QString &taction, const ActionReply &reply)
{
    if (taction == action) {
        emit q->actionPerformed(reply);
    }
}

void ActionWatcher::Private::progressStepSlot(const QString &taction, int i)
{
    if (taction == action) {
        emit q->progressStep(i);
    }
}

void ActionWatcher::Private::progressStepSlot(const QString &taction, const QVariantMap &data)
{
    if (taction == action) {
        emit q->progressStep(data);
    }
}

void ActionWatcher::Private::statusChangedSlot(const QString &taction, Action::AuthStatus status)
{
    if (taction == action) {
        emit q->statusChanged(status);
    }
}

} // namespace Auth

#include "kauthactionwatcher.moc"
