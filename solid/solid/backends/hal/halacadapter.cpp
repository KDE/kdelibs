/*
    Copyright 2006 Kevin Ottens <ervin@kde.org>

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

#include "halacadapter.h"

using namespace Solid::Backends::Hal;

AcAdapter::AcAdapter(HalDevice *device)
    : DeviceInterface(device)
{
    connect(device, SIGNAL(propertyChanged(const QMap<QString,int> &)),
             this, SLOT(slotPropertyChanged(const QMap<QString,int> &)));
}

AcAdapter::~AcAdapter()
{

}

bool AcAdapter::isPlugged() const
{
    return m_device->prop("ac_adapter.present").toBool();
}

void AcAdapter::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("ac_adapter.present"))
    {
        emit plugStateChanged(isPlugged(), m_device->udi());
    }
}

#include "backends/hal/halacadapter.moc"
