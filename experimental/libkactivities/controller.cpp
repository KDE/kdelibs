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

#include "controller.h"
#include "consumer_p.h"
#include "manager_p.h"

#include <QObject>

#include <kdebug.h>

namespace Activities {

class ControllerPrivate: public QObject {
public:
    ControllerPrivate(Controller * parent)
        : q(parent)
    {
    }

private:
    Controller * const q;
};

Controller::Controller(QObject * parent)
    : Consumer(parent), d(new ControllerPrivate(this))
{
    connect(Manager::self(), SIGNAL(ActivityAdded(QString)),
            this, SIGNAL(activityAdded(QString)));

    connect(Manager::self(), SIGNAL(ActivityRemoved(QString)),
            this, SIGNAL(activityRemoved(QString)));

}

Controller::~Controller()
{
    delete d;
}

void Controller::setActivityName(const QString & id, const QString & name)
{
    Manager::self()->SetActivityName(id, name);
}

void Controller::setActivityIcon(const QString & id, const QString & icon)
{
    Manager::self()->SetActivityIcon(id, icon);
}

bool Controller::setCurrentActivity(const QString & id)
{
    return Manager::self()->SetCurrentActivity(id);
}

QString Controller::addActivity(const QString & name)
{
    return Manager::self()->AddActivity(name);
}

void Controller::removeActivity(const QString & id)
{
    Manager::self()->RemoveActivity(id);
}

void Controller::stopActivity(const QString & id)
{
    Manager::self()->StopActivity(id);
}

void Controller::startActivity(const QString & id)
{
    Manager::self()->StartActivity(id);
}

} // namespace Activities

#include "controller.moc"
