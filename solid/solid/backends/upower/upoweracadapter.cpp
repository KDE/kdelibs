/*  This file is part of the KDE project
    Copyright (C) 2009 Pino Toscano <pino@kde.org>
                  2010 Lukas Tinkl <ltinkl@redhat.com>

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
    return m_device->property("Online").toBool();
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
