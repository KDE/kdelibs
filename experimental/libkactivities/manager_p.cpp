/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "manager_p.h"

#include <QDBusConnection>

#include <ktoolinvocation.h>
#include <kdebug.h>

namespace Activities {

Manager * Manager::s_instance = NULL;

// #define ACTIVITY_MANAGER_DBUS_PATH   "org.kde.ActivityManager"
#define ACTIVITY_MANAGER_DBUS_PATH   "org.kde.kactivitymanagerd"
#define ACTIVITY_MANAGER_DBUS_OBJECT "/ActivityManager"

Manager::Manager()
    : org::kde::ActivityManager(
            ACTIVITY_MANAGER_DBUS_PATH,
            ACTIVITY_MANAGER_DBUS_OBJECT,
            QDBusConnection::sessionBus()
            )
{
    connect(&m_watcher, SIGNAL(serviceOwnerChanged(const QString &, const QString &, const QString &)),
            this, SLOT(serviceOwnerChanged(const QString &, const QString &, const QString &)));
}

Manager * Manager::self()
{
    if (!s_instance) {
        // check if the activity manager is already running
        if (!isActivityServiceRunning()) {

            // not running, trying to launch it
            QString error;

            int ret = KToolInvocation::startServiceByDesktopPath("kactivitymanagerd.desktop", QStringList(), &error);
            if (ret > 0) {
                kDebug() << "Activity: Couldn't start kactivitymanagerd: " << error << endl;
            }

            if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(ACTIVITY_MANAGER_DBUS_PATH)) {
                kDebug() << "Activity: The kactivitymanagerd service is still not registered";
            } else {
                kDebug() << "Activity: The kactivitymanagerd service has been registered";
            }
        }

        // creating a new instance of the class
        s_instance = new Manager();
    }

    return s_instance;
}

bool Manager::isActivityServiceRunning()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered(ACTIVITY_MANAGER_DBUS_PATH);
}

void Manager::serviceOwnerChanged(const QString & serviceName, const QString & oldOwner, const QString & newOwner)
{
    Q_UNUSED(oldOwner)

    if (serviceName == ACTIVITY_MANAGER_DBUS_PATH) {
        emit presenceChanged(!newOwner.isEmpty());
    }
}

} // namespace Activities

