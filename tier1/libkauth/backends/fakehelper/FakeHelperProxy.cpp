/*
*   Copyright (C) 2010 Dario Freddi <drf@kde.org>
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

#include "FakeHelperProxy.h"

namespace KAuth {

FakeHelperProxy::FakeHelperProxy()
        : HelperProxy()
{

}

FakeHelperProxy::~FakeHelperProxy()
{

}

void FakeHelperProxy::sendProgressStep(const QVariantMap& step)
{
    Q_UNUSED(step)
}

void FakeHelperProxy::sendProgressStep(int step)
{
    Q_UNUSED(step)
}

void FakeHelperProxy::sendDebugMessage(int level, const char* msg)
{
    Q_UNUSED(level)
    Q_UNUSED(msg)
}

bool FakeHelperProxy::hasToStopAction()
{
    return false;
}

void FakeHelperProxy::setHelperResponder(QObject* o)
{
    Q_UNUSED(o)
}

bool FakeHelperProxy::initHelper(const QString& name)
{
    Q_UNUSED(name)
    return false;
}

void FakeHelperProxy::stopAction(const QString& action, const QString& helperID)
{
    Q_UNUSED(action)
    Q_UNUSED(helperID)
}

KAuth::ActionReply FakeHelperProxy::executeAction(const QString& action, const QString& helperID, const QVariantMap& arguments)
{
    Q_UNUSED(action)
    Q_UNUSED(helperID)
    Q_UNUSED(arguments)
    return KAuth::ActionReply::NoSuchActionReply;
}

bool FakeHelperProxy::executeActions(const QList< QPair< QString, QVariantMap > >& list, const QString& helperID)
{
    Q_UNUSED(list)
    Q_UNUSED(helperID)
    return false;
}

Action::AuthStatus FakeHelperProxy::authorizeAction(const QString& action, const QString& helperID)
{
    Q_UNUSED(action)
    Q_UNUSED(helperID)
    return Action::Denied;
}

}

#include "FakeHelperProxy.moc"
