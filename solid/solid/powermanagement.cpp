/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "powermanagement.h"
#include "powermanagement_p.h"

#include "soliddefs_p.h"
#include "managerbase_p.h"

#include <kglobal.h>
#include <klocale.h>

K_GLOBAL_STATIC(Solid::PowerManagementPrivate, globalPowerManager)

Solid::PowerManagementPrivate::PowerManagementPrivate()
{
}

Solid::PowerManagementPrivate::~PowerManagementPrivate()
{
}

Solid::PowerManagement::AcAdapterState Solid::PowerManagement::acAdapterState()
{
    return UnknownAcAdapterState;
}

QList<Solid::PowerManagement::Action> Solid::PowerManagement::supportedActions()
{
    return QList<Action>();
}

QString Solid::PowerManagement::stringForAction(Action action)
{
    switch (action)
    {
    case StandbyAction:
        return i18n("Standby");
    case SuspendAction:
        return i18n("Suspend");
    case HibernateAction:
        return i18n("Hibernate");
    }
    return QString();
}

void Solid::PowerManagement::request(Action action, QObject *receiver, const char *member)
{

}

bool Solid::PowerManagement::inhibitAutoActions(bool inhibit, const QString &reason)
{
    return false;
}

Solid::PowerManagement::Notifier *Solid::PowerManagement::notifier()
{
    return globalPowerManager;
}

#include "powermanagement_p.moc"
#include "powermanagement.moc"

