/*
*   Copyright (C) 2012 Dario Freddi <drf@kde.org>
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

#include "TestHelper.h"

#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <unistd.h>
#include <QEventLoop>


ActionReply TestHelper::echoaction(QVariantMap args)
{
    qDebug() << "Echo action running";
    ActionReply reply = ActionReply::SuccessReply();
    reply.setData(args);

    return reply;
}

ActionReply TestHelper::standardaction(QVariantMap args)
{
    qDebug() << "Standard action running";
    if (args.contains(QLatin1String("fail")) && args[QLatin1String("fail")].toBool()) {
        return ActionReply::HelperErrorReply();
    }

    return ActionReply::SuccessReply();
}

ActionReply TestHelper::longaction(QVariantMap args)
{
    Q_UNUSED(args);
    qDebug() << "Long action running. Don't be scared, this action takes 2 seconds to complete";

    for (int i = 1; i <= 100; i++) {
        if (HelperSupport::isStopped())
            break;
        if (i == 50) {
             QVariantMap map;
             map.insert(QLatin1String("Answer"), 42);
             HelperSupport::progressStep(map);
        }
        HelperSupport::progressStep(i);
        usleep(20000);
    }

    return ActionReply::SuccessReply();
}
