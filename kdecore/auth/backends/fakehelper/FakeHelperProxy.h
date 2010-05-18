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

#ifndef FAKEHELPERPROXY_H
#define FAKEHELPERPROXY_H

#include "HelperProxy.h"

namespace KAuth {

class FakeHelperProxy : public HelperProxy
{
    Q_OBJECT
    Q_INTERFACES(KAuth::HelperProxy)

    public:
        FakeHelperProxy();
        virtual ~FakeHelperProxy();

        virtual void sendProgressStep(const QVariantMap& step);
        virtual void sendProgressStep(int step);
        virtual void sendDebugMessage(int level, const char* msg);
        virtual bool hasToStopAction();
        virtual void setHelperResponder(QObject* o);
        virtual bool initHelper(const QString& name);
        virtual void stopAction(const QString& action, const QString& helperID);
        virtual ActionReply executeAction(const QString& action, const QString& helperID, const QVariantMap& arguments);
        virtual bool executeActions(const QList< QPair< QString, QVariantMap > >& list, const QString& helperID);
        virtual Action::AuthStatus authorizeAction(const QString& action, const QString& helperID);
};

}

#endif // FAKEHELPERPROXY_H
