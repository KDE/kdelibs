/***************************************************************************
 *   Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kdynamicjobtracker.h"

#include "kuiserverjobtracker.h"
#include "kwidgetjobtracker.h"

#include <kjobtrackerinterface.h>
#include <kdebug.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMap>

class KDynamicJobTracker::Private
{
public:
    Private() : kuiserverTracker(0),
                widgetTracker(0)
    {
    }

    ~Private()
    {
        delete kuiserverTracker;
        delete widgetTracker;
    }

    KUiServerJobTracker *kuiserverTracker;
    KWidgetJobTracker *widgetTracker;
    QMap<KJob*, KJobTrackerInterface*> trackers;
};

KDynamicJobTracker::KDynamicJobTracker(QObject *parent)
    : KJobTrackerInterface(parent),
      d(new Private)
{
}

KDynamicJobTracker::~KDynamicJobTracker()
{
    delete d;
}

void KDynamicJobTracker::registerJob(KJob *job)
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.JobViewServer")) {
        if (!d->kuiserverTracker) {
            d->kuiserverTracker = new KUiServerJobTracker();
        }
        d->trackers[job] = d->kuiserverTracker;
    } else {
        if (!d->widgetTracker) {
            d->widgetTracker = new KWidgetJobTracker();
        }
        d->trackers[job] = d->widgetTracker;
    }

    Q_ASSERT(d->trackers[job]);

    d->trackers[job]->registerJob(job);
}


void KDynamicJobTracker::unregisterJob(KJob *job)
{
    Q_ASSERT(d->trackers[job]);

    d->trackers[job]->unregisterJob(job);
}

#include "kdynamicjobtracker.moc"
