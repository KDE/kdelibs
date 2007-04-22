/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>
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

#include "soliddefs_p.h"
#include "networking_p.h"
#include "networking.h"

#include <kglobal.h>

K_GLOBAL_STATIC(Solid::NetworkingPrivate, globalNetworkManager)

Solid::NetworkingPrivate::NetworkingPrivate()
{
}

Solid::NetworkingPrivate::~NetworkingPrivate()
{
}

bool Solid::Networking::isNetworkingEnabled()
{
    return false;
}

void Solid::Networking::setNetworkingEnabled(bool enabled)
{

}

Solid::Networking::Notifier *Solid::Networking::notifier()
{
    return globalNetworkManager;
}

#include "networking_p.moc"
#include "networking.moc"
