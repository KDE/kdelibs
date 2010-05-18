/*
*   Copyright (C) 2009 Nicola Gigante <nicola.gigante@gmail.com>
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

#include "kauthaction.h"

#include <QDebug>
#include <QPluginLoader>
#include <QRegExp>

#include "BackendsManager.h"
#include "kauthactionwatcher.h"

namespace KAuth
{

class Action::Private
{
public:
    Private() : valid(false), async(false) {}

    QString name;
    QString details;
    QString helperId;
    QVariantMap args;
    bool valid;
    bool async;
};

// Constructors
Action::Action()
        : d(new Private())
{
}

Action::Action(const Action &action)
        : d(new Private())
{
    *this = action;
}

Action::Action(const QString &name)
        : d(new Private())
{
    setName(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::Action(const QString &name, const QString &details)
        : d(new Private())
{
    setName(name);
    setDetails(details);
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

bool Action::operator==(const Action &action) const
{
    return d->name == action.d->name;
}

bool Action::operator!=(const Action &action) const
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
    d->name = name;

    // Does the backend support checking for known actions?
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::CheckActionExistenceCapability) {
        // In this case, just ask the backend
        d->valid = BackendsManager::authBackend()->actionExists(name);
    } else {
        // Otherwise, check through a regexp
        QRegExp exp("[0-z]+(\\.[0-z]+)*");
        d->valid = exp.exactMatch(name);
    }
}

QString Action::details() const
{
    return d->details;
}

void Action::setDetails(const QString &details)
{
    d->details = details;
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

QString Action::helperID() const
{
    return d->helperId;
}

// TODO: Check for helper id's syntax
void Action::setHelperID(const QString &id)
{
    d->helperId = id;
}

// Authorizaton methods
Action::AuthStatus Action::authorize() const
{
    if (!isValid()) {
        return Action::Invalid;
    }

    // Let's check capabilities
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
        // That's easy then
        return BackendsManager::authBackend()->authorizeAction(d->name);
    } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
        // We need to check if we have an helper in this case
        if (hasHelper()) {
            // Ok, we need to use "helper authorization".
            return BackendsManager::helperProxy()->authorizeAction(d->name, d->helperId);
        } else {
            // Ok, in this case we have to fake and just pretend we are an helper
            if (BackendsManager::authBackend()->isCallerAuthorized(d->name, BackendsManager::authBackend()->callerID())) {
                return Authorized;
            } else {
                return Denied;
            }
        }
    } else {
        // This should never, never happen
        return Invalid;
    }
}


Action::AuthStatus Action::earlyAuthorize() const
{
    // Check the status first
    AuthStatus s = status();
    if (s == AuthRequired) {
        // Let's check what to do
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
            // In this case we can actually try an authorization
            return BackendsManager::authBackend()->authorizeAction(d->name);
        } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
            // In this case, just throw out Authorized, as the auth will take place later
            return Authorized;
        } else {
            // This should never, never happen
            return Invalid;
        }
    } else {
        // It's fine, return the status
        return s;
    }
}


Action::AuthStatus Action::status() const
{
    if (!isValid()) {
        return Action::Invalid;
    }

    return BackendsManager::authBackend()->actionStatus(d->name);
}

// Execution methods
bool Action::executeActions(const QList<Action> &actions, QList<Action> *deniedActions, const QString &helperID)
{
    QList<QPair<QString, QVariantMap> > list;

    foreach(const Action &a, actions) {
        // Save us an additional step
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
            AuthStatus s = BackendsManager::authBackend()->authorizeAction(a.name());

            if (s == Authorized) {
                list.push_back(QPair<QString, QVariantMap>(a.name(), a.arguments()));
            } else if ((s == Denied || s == Invalid) && deniedActions) {
                *deniedActions << a;
            }
        } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
            list.push_back(QPair<QString, QVariantMap>(a.name(), a.arguments()));
        } else {
            // There's something totally wrong here
            return false;
        }
    }

    if (list.isEmpty()) {
        return false;
    }

    return BackendsManager::helperProxy()->executeActions(list, helperID);
}

bool Action::executesAsync() const
{
    return d->async;
}

void Action::setExecutesAsync(bool async)
{
    d->async = async;
}

ActionReply Action::execute() const
{
    if (!isValid())
        return ActionReply::InvalidActionReply;

    return execute(helperID());
}

ActionReply Action::execute(const QString &helperID) const
{
    // Is the action valid?
    if (!isValid()) {
        return Invalid;
    }

    // What to do?
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
        // Authorize from here
        AuthStatus s = BackendsManager::authBackend()->authorizeAction(d->name);

        // Abort if authorization fails
        switch (s) {
        case Denied:
            return ActionReply::AuthorizationDeniedReply;
        case Invalid:
            return ActionReply::InvalidActionReply;
        case UserCancelled:
            return ActionReply::UserCancelledReply;
        default:
            break;
        }
    } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
        // In this case we care only if the action is not async and does not have an helper
        if (!d->async && !hasHelper()) {
            // Authorize!
            switch (authorize()) {
            case Denied:
                return ActionReply::AuthorizationDeniedReply;
            case Invalid:
                return ActionReply::InvalidActionReply;
            case UserCancelled:
                return ActionReply::UserCancelledReply;
            default:
                break;
            }
        }
    } else {
        // What?
        return Invalid;
    }

    if (d->async) {
        if (hasHelper()) {
            // It makes no sense
            return Invalid;
        }

        return executeActions(QList<Action>() << *this, NULL, helperID) ?
            ActionReply::SuccessReply : ActionReply::AuthorizationDeniedReply;
    } else {
        if (hasHelper()) {
            return BackendsManager::helperProxy()->executeAction(d->name, helperID, d->args);
        } else {
            return ActionReply::SuccessReply;
        }
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

bool Action::hasHelper() const
{
    return !d->helperId.isEmpty();
}

} // namespace Auth
