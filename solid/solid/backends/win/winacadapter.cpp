/*
    Copyright 2013 Patrick von Reth <vonreth@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/
#include "winacadapter.h"

using namespace Solid::Backends::Win;

WinAcadapter::WinAcadapter(WinDevice *device) :
    WinInterface(device)
{
    powerChanged();
    connect(WinDeviceManager::instance(),SIGNAL(powerChanged()),this,SLOT(powerChanged()));
}

bool WinAcadapter::isPlugged() const
{
    return m_pluggedIn == 1;
}

QSet<QString> WinAcadapter::getUdis()
{
    QSet<QString> out;
    out << QLatin1String("/org/kde/solid/win/power/acadapter");
    return out;
}

void WinAcadapter::powerChanged()
{
    SYSTEM_POWER_STATUS status;
    ZeroMemory(&status,sizeof(SYSTEM_POWER_STATUS));
    GetSystemPowerStatus(&status);
    if(status.ACLineStatus != m_pluggedIn)
    {
        emit plugStateChanged(status.ACLineStatus == 1,m_device->udi());
    }
    m_pluggedIn = status.ACLineStatus;

}

#include "winacadapter.moc"
