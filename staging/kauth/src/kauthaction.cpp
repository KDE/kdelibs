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

#include "kauthaction.h"

#include <QtGlobal>
#include <QRegExp>
#include <QWidget>

#include "kauthexecutejob.h"

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

QString Action::helperId() const
{
    return d->helperId;
}

// TODO: Check for helper id's syntax
void Action::setHelperId(const QString &id)
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

Action::AuthStatus Action::status() const
{
    if (!isValid()) {
        return Action::InvalidStatus;
    }

    return BackendsManager::authBackend()->actionStatus(d->name);
}

ExecuteJob *Action::execute(ExecutionMode mode)
{
    return new ExecuteJob(*this, mode, 0);
}

bool Action::hasHelper() const
{
    return !d->helperId.isEmpty();
}

} // namespace Auth
