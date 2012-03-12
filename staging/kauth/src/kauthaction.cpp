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

#include <QtGlobal>
#include <QRegExp>
#include <QWidget>

#include "BackendsManager.h"

namespace KAuth
{

class ActionData : public QSharedData
{
public:
    ActionData() : valid(false), parent(0) {}
    ActionData(const ActionData &other)
        : QSharedData(other)
        , name(other.name)
        , details(other.details)
        , helperId(other.helperId)
        , args(other.args)
        , valid(other.valid)
        , parent(other.parent) {}
    ~ActionData() {}

    QString name;
    QString details;
    QString helperId;
    QVariantMap args;
    bool valid;
    QWidget *parent;
};

// Constructors
Action::Action()
    : d(new ActionData())
{
}

Action::Action(const Action &action)
    : d(action.d)
{
}

Action::Action(const QString &name)
        : d(new ActionData())
{
    setName(name);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::Action(const QString &name, const QString &details)
        : d(new ActionData())
{
    setName(name);
    setDetails(details);
    BackendsManager::authBackend()->setupAction(d->name);
}

Action::~Action()
{
}

// Operators
Action &Action::operator=(const Action &action)
{
    if (this == &action) {
        // Protect against self-assignment
        return *this;
    }

    d = action.d;
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
        QRegExp exp(QLatin1String("[0-z]+(\\.[0-z]+)*"));
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

QString Action::helperID() const
{
    return d->helperId;
}

// TODO: Check for helper id's syntax
void Action::setHelperID(const QString &id)
{
    d->helperId = id;
}

void Action::setParentWidget(QWidget* parent)
{
    d->parent = parent;
}

QWidget* Action::parentWidget() const
{
    return d->parent;
}


// Authorizaton methods
Action::AuthStatus Action::authorize() const
{
    if (!isValid()) {
        return Action::StatusInvalid;
    }

    // If there is any pre auth action, let's perform it
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
        BackendsManager::authBackend()->preAuthAction(d->name, d->parent);
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
                return StatusAuthorized;
            } else {
                return StatusDenied;
            }
        }
    } else {
        // This should never, never happen
        return StatusInvalid;
    }
}


Action::AuthStatus Action::earlyAuthorize() const
{
    // Check the status first
    AuthStatus s = status();
    if (s == StatusAuthRequired) {
        // Let's check what to do
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
            // In this case we can actually try an authorization
            if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
                BackendsManager::authBackend()->preAuthAction(d->name, d->parent);
            }

            return BackendsManager::authBackend()->authorizeAction(d->name);
        } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
            // In this case, just throw out Authorized, as the auth will take place later
            return StatusAuthorized;
        } else {
            // This should never, never happen
            return StatusInvalid;
        }
    } else {
        // It's fine, return the status
        return s;
    }
}


Action::AuthStatus Action::status() const
{
    if (!isValid()) {
        return Action::StatusInvalid;
    }

    return BackendsManager::authBackend()->actionStatus(d->name);
}

ActionReply Action::execute() const
{
    return execute(helperID());
}

ActionReply Action::execute(const QString &helperID) const
{
    // Is the action valid?
    if (!isValid()) {
        return ActionReply::InvalidActionReply;
    }

    // What to do?
    if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromClientCapability) {
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
            BackendsManager::authBackend()->preAuthAction(d->name, d->parent);
        }
        // Authorize from here
        AuthStatus s = BackendsManager::authBackend()->authorizeAction(d->name);

        // Abort if authorization fails
        switch (s) {
        case StatusDenied:
            return ActionReply::AuthorizationDeniedReply;
        case StatusInvalid:
            return ActionReply::InvalidActionReply;
        case StatusUserCancelled:
            return ActionReply::UserCancelledReply;
        default:
            break;
        }
    } else if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::AuthorizeFromHelperCapability) {
        // In this case we care only if the action does not have an helper
        if (!hasHelper()) {
            // Authorize!
            switch (authorize()) {
            case StatusDenied:
                return ActionReply::AuthorizationDeniedReply;
            case StatusInvalid:
                return ActionReply::InvalidActionReply;
            case StatusUserCancelled:
                return ActionReply::UserCancelledReply;
            default:
                break;
            }
        }
    } else {
        // What?
        return ActionReply::InvalidActionReply;
    }

    if (hasHelper()) {
        // Perform the pre auth here
        if (BackendsManager::authBackend()->capabilities() & KAuth::AuthBackend::PreAuthActionCapability) {
            BackendsManager::authBackend()->preAuthAction(d->name, d->parent);
        }

        return BackendsManager::helperProxy()->executeAction(d->name, helperID, d->args, false);
    } else {
        return ActionReply::SuccessReply;
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
