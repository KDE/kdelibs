/*
*   Copyright (C) 2009-2012 Dario Freddi <drf@kde.org>
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

#include "kauthactionreply.h"

#include <QDebug>

namespace KAuth
{

class ActionReplyData : public QSharedData
{
public:
    ActionReplyData() {}
    ActionReplyData(const ActionReplyData &other)
        : QSharedData(other)
        , data(other.data)
        , errorCode(other.errorCode)
        , errorDescription(errorDescription)
        , type(type) {}
    ~ActionReplyData() {}

    QVariantMap data; // User-defined data for success and helper error replies, empty for kauth errors
    uint errorCode;
    QString errorDescription;
    ActionReply::Type type;
};

// Predefined replies
const ActionReply ActionReply::SuccessReply() { return ActionReply(); }
const ActionReply ActionReply::HelperErrorReply() { return ActionReply(ActionReply::HelperErrorType); }
const ActionReply ActionReply::NoResponderReply() { return ActionReply(ActionReply::NoResponderError); }
const ActionReply ActionReply::NoSuchActionReply() { return ActionReply(ActionReply::NoSuchActionError); }
const ActionReply ActionReply::InvalidActionReply() { return ActionReply(ActionReply::InvalidActionError); }
const ActionReply ActionReply::AuthorizationDeniedReply() { return ActionReply(ActionReply::AuthorizationDeniedError); }
const ActionReply ActionReply::UserCancelledReply() { return ActionReply(ActionReply::UserCancelledError); }
const ActionReply ActionReply::HelperBusyReply() { return ActionReply(ActionReply::HelperBusyError); }
const ActionReply ActionReply::AlreadyStartedReply() { return ActionReply(ActionReply::AlreadyStartedError); }
const ActionReply ActionReply::DBusErrorReply() { return ActionReply(ActionReply::DBusError); }

// Constructors
ActionReply::ActionReply(const ActionReply &reply)
    : d(reply.d)
{
}

ActionReply::ActionReply()
        : d(new ActionReplyData())
{
    d->errorCode = 0;
    d->type = SuccessType;
}

ActionReply::ActionReply(ActionReply::Type type)
        : d(new ActionReplyData())
{
    d->errorCode = 0;
    d->type = type;
}

ActionReply::ActionReply(int error)
        : d(new ActionReplyData())
{
    d->errorCode = error;
    d->type = KAuthErrorType;
}

ActionReply::~ActionReply()
{
}

void ActionReply::setData(const QVariantMap &data)
{
    d->data = data;
}

void ActionReply::addData(const QString &key, const QVariant &value)
{
    d->data.insert(key, value);
}

QVariantMap ActionReply::data() const
{
    return d->data;
}

ActionReply::Type ActionReply::type() const
{
    return d->type;
}

void ActionReply::setType(ActionReply::Type type)
{
    d->type = type;
}

bool ActionReply::succeeded() const
{
    return d->type == SuccessType;
}

bool ActionReply::failed() const
{
    return !succeeded();
}

ActionReply::Error ActionReply::errorCode() const
{
    return (ActionReply::Error)d->errorCode;
}

void ActionReply::setErrorCode(Error errorCode)
{
    d->errorCode = errorCode;
    if (d->type != HelperErrorType) {
        d->type = KAuthErrorType;
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

ActionReply ActionReply::deserialize(const QByteArray &data)
{
    ActionReply reply;
    QByteArray a(data);
    QDataStream s(&a, QIODevice::ReadOnly);

    s >> reply;

    return reply;
}

// Operators
ActionReply &ActionReply::operator=(const ActionReply &reply)
{
    if (this == &reply) {
        // Protect against self-assignment
        return *this;
    }

    d = reply.d;
    return *this;
}

bool ActionReply::operator==(const ActionReply &reply) const
{
    return (d->type == reply.d->type && d->errorCode == reply.d->errorCode);
}

bool ActionReply::operator!=(const ActionReply &reply) const
{
    return (d->type != reply.d->type || d->errorCode != reply.d->errorCode);
}

QDataStream &operator<<(QDataStream &d, const ActionReply &reply)
{
    return d << reply.d->data << reply.d->errorCode << (quint32)reply.d->type << reply.d->errorDescription;
}

QDataStream &operator>>(QDataStream &stream, ActionReply &reply)
{
    quint32 i;
    stream >> reply.d->data >> reply.d->errorCode >> i >> reply.d->errorDescription;
    reply.d->type = (ActionReply::Type) i;

    return stream;
}

} // namespace Auth
