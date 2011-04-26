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

#ifndef ACTIVITY_MANAGER_P_
#define ACTIVITY_MANAGER_P_

#include "activitymanager_interface.h"

#include <QDBusServiceWatcher>

class KActivityManager: public org::kde::ActivityManager {
    Q_OBJECT
public:
    static KActivityManager * self();

    static bool isActivityServiceRunning();

public Q_SLOTS:
    void serviceOwnerChanged(const QString & serviceName, const QString & oldOwner, const QString & newOwner);

Q_SIGNALS:
    void presenceChanged(bool present);

private:
    KActivityManager();

    QDBusServiceWatcher m_watcher;

    static KActivityManager * s_instance;
};


#endif // ACTIVITY_MANAGER_P_
