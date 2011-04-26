/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kactivityconsumer.h"
#include "kactivityconsumer_p.h"
#include "kactivitymanager_p.h"

#include <kdebug.h>

KActivityConsumer::KActivityConsumer(QObject * parent)
    : QObject(parent), d(new KActivityConsumerPrivate())
{
    connect(
        KActivityManager::self(), SIGNAL(CurrentActivityChanged(const QString &)),
        this,                     SIGNAL(currentActivityChanged(const QString &))
    );
}

KActivityConsumer::~KActivityConsumer()
{
    delete d;
}

// macro defines a shorthand for validating and returning a d-bus result
// @param TYPE type of the result
// @param METHOD invocation of the d-bus method
// @param DEFAULT value to be used if the reply was not valid
#define KACTIVITYCONSUMER_DBUS_RETURN(TYPE, METHOD, DEFAULT)  \
    QDBusReply < TYPE > dbusReply = METHOD;                   \
    if (dbusReply.isValid()) {                                \
        return dbusReply.value();                             \
    } else {                                                  \
        kDebug() << "d-bus reply was invalid"                 \
                 << dbusReply.value()                         \
                 << dbusReply.error();                        \
        return DEFAULT;                                       \
    }

QString KActivityConsumer::currentActivity() const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QString, KActivityManager::self()->CurrentActivity(), QString() );
}

QStringList KActivityConsumer::listActivities(KActivityInfo::State state) const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, KActivityManager::self()->ListActivities(state), QStringList() );
}

QStringList KActivityConsumer::listActivities() const
{
    KACTIVITYCONSUMER_DBUS_RETURN(
        QStringList, KActivityManager::self()->ListActivities(), QStringList() );
}

#undef KACTIVITYCONSUMER_DBUS_RETURN

KActivityConsumer::ServiceStatus KActivityConsumer::serviceStatus()
{
    if (!KActivityManager::isActivityServiceRunning()) {
        return NotRunning;
    }

    if (!KActivityManager::self()->IsBackstoreAvailable()) {
        return BareFunctionality;
    }

    return FullFunctionality;
}

