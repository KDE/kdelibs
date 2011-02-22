/*
    Copyright 2009 Pino Toscano <pino@kde.org>
    Copyright 2010 Lukas Tinkl <ltinkl@redhat.com>

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

#include "upoweracadapter.h"

using namespace Solid::Backends::UPower;

AcAdapter::AcAdapter(UPowerDevice *device)
    : DeviceInterface(device)
{
    connect(device, SIGNAL(changed()), this, SLOT(slotChanged()));

    updateCache();
}

AcAdapter::~AcAdapter()
{
}

bool AcAdapter::isPlugged() const
{
    return m_device->prop("Online").toBool();
}

void AcAdapter::slotChanged()
{
    const bool old_isPlugged = m_isPlugged;
    updateCache();

    if (old_isPlugged != m_isPlugged)
    {
        emit plugStateChanged(m_isPlugged, m_device->udi());
    }
}

void AcAdapter::updateCache()
{
    m_isPlugged = isPlugged();
}

#include "backends/upower/upoweracadapter.moc"
