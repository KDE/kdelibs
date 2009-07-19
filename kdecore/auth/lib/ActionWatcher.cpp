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

#include "ActionWatcher.h"
#include "BackendsManager.h"

#include <QHash>

namespace Auth
{

class ActionWatcher::Private
{
public:
    QString action;
};

static QHash<QString, ActionWatcher *> s_watchers;

ActionWatcher::ActionWatcher(const QString &action) : QObject(NULL)
{
    d = new Private;
    d->action = action;

    HelperProxy *helper = BackendsManager::helperProxy();

    connect(helper, SIGNAL(actionStarted(QString)), this, SLOT(actionStartedSlot(QString)));
    connect(helper, SIGNAL(actionPerformed(QString, ActionReply)), this, SLOT(actionPerformedSlot(QString, ActionReply)));
    connect(helper, SIGNAL(progressStep(QString, int)), this, SLOT(progressStepSlot(QString, int)));
    connect(helper, SIGNAL(progressStep(QString, QVariantMap)), this, SLOT(progressStepSlot(QString, QVariantMap)));
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

void ActionWatcher::actionStartedSlot(const QString &action)
{
    if(d->action == action)
        emit actionStarted();
}

void ActionWatcher::actionPerformedSlot(const QString &action, ActionReply reply)
{
    if(d->action == action)
        emit actionPerformed(reply);
}

void ActionWatcher::progressStepSlot(const QString &action, int i)
{
    if(d->action == action)
        emit progressStep(i);
}

void ActionWatcher::progressStepSlot(const QString &action, QVariantMap data)
{
    if(d->action == action)
        emit progressStep(data);
}

} // namespace Auth