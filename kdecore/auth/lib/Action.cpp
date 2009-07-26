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

#include "Action.h"

#include <QDebug>
#include <QPluginLoader>
#include <QRegExp>

#include "BackendsManager.h"

namespace KAuth
{

class Action::Private
{
public:
    Private() : valid(false) {}
    
    QString name;
    QVariantMap args;
    bool valid;
};

static QString s_helperID;

// Constructors
Action::Action()
{
    d = new Private();
}

Action::Action(const Action &action)
{
    d = new Private();
    
    setName(action.d->name);
    d->args = action.d->args;
}

Action::Action(const char *name)
{
    d = new Private();
    
    setName(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::Action(const QString &name)
{
    d = new Private();
    
    setName(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::~Action()
{
    delete d;
}

// Operators
Action &Action::operator=(const Action & action)
{
    setName(action.d->name);
    d->args = action.d->args;

    return *this;
}

bool Action::operator==(const Action &action)
{
    return d->name == action.d->name;
}

bool Action::operator!=(const Action &action)
{
    return d->name != action.d->name;
}

// Accessors
QString Action::name() const
{
    return d->name;
}

void Action::setName(const QString &name)
{
    QRegExp exp("[a-z]+(\\.[a-z]+)*");
    
    d->name = name;
    d->valid = exp.exactMatch(name);
}

bool Action::isValid() const
{
    return d->valid;
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
    if(!isValid())
        return Action::Invalid;
    
    return BackendsManager::authBackend()->authorizeAction(d->name);
}

Action::AuthStatus Action::status() const
{
    if(!isValid())
        return Action::Invalid;
    
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
        } else if ((s == Denied || s == Invalid) && deniedActions) {
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
    if(!isValid())
        return Action::Invalid;
    
    return executeAsync(helperID(), target, slot);
}

Action::AuthStatus Action::executeAsync(const QString &helperID, QObject *target, const char *slot)
{
    AuthStatus s = authorize();
    if (s == Denied || s == UserCancelled || s == Invalid) {
        return s;
    }

    if (target && slot) {
        QObject::connect(watcher(), SIGNAL(actionPerformed(ActionReply)), target, slot);
    }

    return executeActions(QList<Action>() << *this, NULL, helperID) ? Action::Authorized : Action::Error;
}

ActionReply Action::execute() const
{
    if(!isValid())
        return ActionReply::InvalidActionReply;
    
    return execute(helperID());
}

ActionReply Action::execute(const QString &helperID) const
{
    AuthStatus s = authorize();
    
    switch(s) {
    case Denied:
        return ActionReply::AuthorizationDeniedReply;
    case Invalid:
        return ActionReply::InvalidActionReply;
    case UserCancelled:
        return ActionReply::UserCancelledReply;
    default:
        return BackendsManager::helperProxy()->executeAction(d->name, helperID, d->args);
    }
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