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

#include "TestHelperProxy.h"

namespace KAuth {

TestHelperProxy::TestHelperProxy()
        : HelperProxy()
{

}

TestHelperProxy::~TestHelperProxy()
{

}

void TestHelperProxy::sendProgressStep(const QVariantMap& step)
{
    Q_UNUSED(step)
}

void TestHelperProxy::sendProgressStep(int step)
{
    Q_UNUSED(step)
}

void TestHelperProxy::sendDebugMessage(int level, const char* msg)
{
    Q_UNUSED(level)
    Q_UNUSED(msg)
}

bool TestHelperProxy::hasToStopAction()
{
    return false;
}

void TestHelperProxy::setHelperResponder(QObject* o)
{
    Q_UNUSED(o)
}

bool TestHelperProxy::initHelper(const QString& name)
{
    Q_UNUSED(name)
    return false;
}

void TestHelperProxy::stopAction(const QString& action, const QString& helperID)
{
    Q_UNUSED(action)
    Q_UNUSED(helperID)
}

void TestHelperProxy::executeAction(const QString& action, const QString& helperID, const QVariantMap& arguments)
{
    Q_UNUSED(action)
    Q_UNUSED(helperID)
    Q_UNUSED(arguments)
}

Action::AuthStatus TestHelperProxy::authorizeAction(const QString& action, const QString& helperID)
{
    Q_UNUSED(action)
    Q_UNUSED(helperID)
    return Action::StatusDenied;
}

}

#include "TestHelperProxy.moc"
