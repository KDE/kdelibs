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

#include "kactivitycontroller.h"
#include "kactivityconsumer_p.h"
#include "kactivitymanager_p.h"

#include <QObject>

#include <kdebug.h>

class KActivityController::Private: public QObject {
public:
    Private(KActivityController * parent)
        : q(parent)
    {
    }

private:
    KActivityController * const q;
};

KActivityController::KActivityController(QObject * parent)
    : KActivityConsumer(parent), d(new Private(this))
{
    connect(KActivityManager::self(), SIGNAL(ActivityAdded(QString)),
            this, SIGNAL(activityAdded(QString)));

    connect(KActivityManager::self(), SIGNAL(ActivityRemoved(QString)),
            this, SIGNAL(activityRemoved(QString)));

}

KActivityController::~KActivityController()
{
    delete d;
}

void KActivityController::setActivityName(const QString & id, const QString & name)
{
    KActivityManager::self()->SetActivityName(id, name);
}

void KActivityController::setActivityIcon(const QString & id, const QString & icon)
{
    KActivityManager::self()->SetActivityIcon(id, icon);
}

bool KActivityController::setCurrentActivity(const QString & id)
{
    return KActivityManager::self()->SetCurrentActivity(id);
}

QString KActivityController::addActivity(const QString & name)
{
    return KActivityManager::self()->AddActivity(name);
}

void KActivityController::removeActivity(const QString & id)
{
    KActivityManager::self()->RemoveActivity(id);
}

void KActivityController::stopActivity(const QString & id)
{
    KActivityManager::self()->StopActivity(id);
}

void KActivityController::startActivity(const QString & id)
{
    KActivityManager::self()->StartActivity(id);
}

#include "kactivitycontroller.moc"
