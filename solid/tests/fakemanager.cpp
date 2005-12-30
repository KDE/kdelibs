/*  This file is part of the KDE project
    Copyright (C) 2005 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <kdebug.h>

#include "fakemanager.h"
#include "fakedevice.h"


FakeManager::FakeManager()
    : DeviceManager()
{
}

FakeManager::~FakeManager()
{

}

QStringList FakeManager::allDevices()
{
    return QStringList();
}

bool FakeManager::deviceExists( const QString &udi )
{
    return false;
}

KDEHW::Ifaces::Device *FakeManager::createDevice( const QString &udi )
{
    return 0L;
}

QStringList FakeManager::findDeviceStringMatch( const QString &key, const QString &value )
{
    return QStringList();
}

QStringList FakeManager::findDeviceByCapability( const QString &capability )
{
    return QStringList();
}

#include "fakemanager.moc"
