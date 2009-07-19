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

#include "ActionReply.h"

#include <QDebug>

namespace Auth
{

class ActionReply::Private
{
public:
    QVariantMap data; // User-defined data for success and helper error replies, empty for kauth errors
    int errorCode;
    QString errorDescription;
    ActionReply::Type type;
};

// Predefined replies
const ActionReply ActionReply::SuccessReply = ActionReply();
const ActionReply ActionReply::HelperErrorReply = ActionReply(ActionReply::HelperError);
const ActionReply ActionReply::NoResponderReply = ActionReply(ActionReply::NoResponder);
const ActionReply ActionReply::NoSuchActionReply = ActionReply(ActionReply::NoSuchAction);
const ActionReply ActionReply::AuthorizationDeniedReply = ActionReply(ActionReply::AuthorizationDenied);
const ActionReply ActionReply::UserCancelledReply = ActionReply(ActionReply::UserCancelled);
const ActionReply ActionReply::HelperBusyReply = ActionReply(ActionReply::HelperBusy);
const ActionReply ActionReply::DBusErrorReply = ActionReply(ActionReply::DBusError);

// Constructors
ActionReply::ActionReply(const ActionReply &reply)
{
    d = new Private;
    d->data = reply.d->data;
    d->errorCode = reply.d->errorCode;
    d->errorDescription = reply.d->errorDescription;
    d->type = reply.d->type;
}

ActionReply::ActionReply()
{
    d = new Private;
    d->errorCode = 0;
    d->type = Success;
}

ActionReply::ActionReply(ActionReply::Type type)
{
    d = new Private;
    d->errorCode = 0;
    d->type = type;
}

ActionReply::ActionReply(int error)
{
    d = new Private;
    d->errorCode = error;
    d->type = KAuthError;
}

ActionReply::~ActionReply()
{
    delete d;
}

QVariantMap &ActionReply::data()
{
    return d->data;
}

QVariantMap ActionReply::data() const
{
    return d->data;
}

ActionReply::Type ActionReply::type() const
{
    return d->type;
}

bool ActionReply::succeeded() const
{
    return d->type == Success;
}

bool ActionReply::failed() const
{
    return d->type != Success;
}

int ActionReply::errorCode() const
{
    return d->errorCode;
}

void ActionReply::setErrorCode(int errorCode)
{
    d->errorCode = errorCode;
    if (d->type != HelperError) {
        d->type = KAuthError;
    }
}

QString ActionReply::errorDescription() const
{
    return d->errorDescription;
}

void ActionReply::setErrorDescription(const QString &error)
{
    d->errorDescription = error;
}

QByteArray ActionReply::serialized() const
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);

    s << *this;

    return data;
}

ActionReply ActionReply::deserialize(QByteArray data)
{
    ActionReply reply;
    QDataStream s(&data, QIODevice::ReadOnly);

    s >> reply;

    return reply;
}

// Operators
ActionReply &ActionReply::operator=(const ActionReply & reply)
{
    d->data = reply.d->data;
    d->errorCode = reply.d->errorCode;
    d->errorDescription = reply.d->errorDescription;
    d->type = reply.d->type;

    return *this;
}

bool ActionReply::operator==(const ActionReply &reply)
{
    return (d->type == reply.d->type && d->errorCode == reply.d->errorCode);
}

bool ActionReply::operator!=(const ActionReply &reply)
{
    return (d->type != reply.d->type || d->errorCode != reply.d->errorCode);
}

QDataStream &operator<<(QDataStream &d, const ActionReply &reply)
{
    return d << reply.d->data << reply.d->errorCode << (quint32)reply.d->type;
}

QDataStream &operator>>(QDataStream &stream, ActionReply &reply)
{
    quint32 i;
    stream >> reply.d->data >> reply.d->errorCode >> i;
    reply.d->type = (ActionReply::Type) i;

    return stream;
}

} // namespace Auth
