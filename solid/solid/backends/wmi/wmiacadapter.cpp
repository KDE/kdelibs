/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#include "wmiacadapter.h"

using namespace Solid::Backends::Wmi;

AcAdapter::AcAdapter(WmiDevice *device)
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
    return m_device->property("ac_adapter.present").toBool();
}

void AcAdapter::slotPropertyChanged(const QMap<QString,int> &changes)
{
    if (changes.contains("ac_adapter.present"))
    {
        emit plugStateChanged(isPlugged(), m_device->udi());
    }
}

#include "backends/wmi/wmiacadapter.moc"
