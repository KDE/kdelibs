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

#include "Action.h"

#include <QDebug>
#include <QPluginLoader>

#include "BackendsManager.h"

namespace KAuth
{

class Action::Private
{
public:
    Private(QString name) : name(name) {}

    QString name;
    QVariantMap args;
};

static QString s_helperID;

// Constructors
Action::Action(const Action &action)
{
    d = new Private(action.d->name);
    d->args = action.d->args;
}

Action::Action(const char *name)
{
    d = new Private(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::Action(const QString &name)
{
    d = new Private(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::~Action()
{
    delete d;
}

// Operators
Action &Action::operator=(const Action & action)
{
    d->name = action.d->name;
    d->args = action.d->args;

    return *this;
}

// Accessors
QString Action::name() const
{
    return d->name;
}

void Action::setName(const QString &name)
{
    d->name = name;
}

void Action::setArguments(const QVariantMap &arguments)
{
    d->args = arguments;
}

void Action::addArgument(const QString &key, const QVariant &value)
{
    d->args.insert(key, value);
}

QVariantMap Action::arguments() const
{
    return d->args;
}

ActionWatcher *Action::watcher()
{
    return ActionWatcher::watcher(d->name);
}

QString Action::helperID()
{
    return s_helperID;
}

// TODO: Check for helper id's syntax
void Action::setHelperID(const QString &id)
{
    s_helperID = id;
}

// Authorizaton methods
Action::AuthStatus Action::authorize() const
{
    return BackendsManager::authBackend()->authorizeAction(d->name);
}

Action::AuthStatus Action::status() const
{
    return BackendsManager::authBackend()->actionStatus(d->name);
}

// Execution methods
bool Action::executeActions(const QList<Action> &actions, QList<Action> *deniedActions, const QString &helperID)
{
    QList<QPair<QString, QVariantMap> > list;

    foreach(const Action &a, actions) {
        AuthStatus s = a.authorize();
        if (s == Authorized) {
            list.push_back(QPair<QString, QVariantMap>(a.name(), a.arguments()));
        } else if (s == Denied && deniedActions) {
            *deniedActions << a;
        }
    }

    if (list.isEmpty()) {
        return false;
    }

    return BackendsManager::helperProxy()->executeActions(list, helperID);
}

Action::AuthStatus Action::executeAsync(QObject *target, const char *slot)
{
    return executeAsync(helperID(), target, slot);
}

Action::AuthStatus Action::executeAsync(const QString &helperID, QObject *target, const char *slot)
{
    AuthStatus s = authorize();
    if (s == Denied || s == UserCancelled) {
        return s;
    }

    if (target && slot) {
        QObject::connect(watcher(), SIGNAL(actionPerformed(ActionReply)), target, slot);
    }

    return executeActions(QList<Action>() << *this, NULL, helperID) ? Action::Authorized : Action::Error;
}

ActionReply Action::execute() const
{
    return execute(helperID());
}

ActionReply Action::execute(const QString &helperID) const
{
    AuthStatus s = authorize();
    if (s == Denied) {
        return ActionReply::AuthorizationDeniedReply;
    } else if(s == UserCancelled) {
        return ActionReply::UserCancelledReply;
    }
    
    return BackendsManager::helperProxy()->executeAction(d->name, helperID, d->args);
}

void Action::stop()
{
    stop(helperID());
}

void Action::stop(const QString &helperID)
{
    BackendsManager::helperProxy()->stopAction(d->name, helperID);
}

} // namespace Auth