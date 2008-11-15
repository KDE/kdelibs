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

#ifndef SOLID_POWERMANAGEMENT_P_H
#define SOLID_POWERMANAGEMENT_P_H

#include "powermanagement.h"

#include "powermanagementinterface.h"
#include "inhibitinterface.h"

namespace Solid
{
    class PowerManagementPrivate : public PowerManagement::Notifier
    {
        Q_OBJECT
    public:
        PowerManagementPrivate();
        ~PowerManagementPrivate();

    public Q_SLOTS:
        void slotCanSuspendChanged(bool newState);
        void slotCanHibernateChanged(bool newState);
        void slotPowerSaveStatusChanged(bool newState);

    public:
        OrgFreedesktopPowerManagementInterface managerIface;
        OrgFreedesktopPowerManagementInhibitInterface inhibitIface;

        bool powerSaveStatus;
        QSet<Solid::PowerManagement::SleepState> supportedSleepStates;
    };
}

#endif
