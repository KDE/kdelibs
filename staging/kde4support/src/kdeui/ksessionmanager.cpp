/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
    Copyright (c) 1998, 1999 KDE Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ksessionmanager.h"

KSessionManager::KSessionManager():
     d(0)
{
    sessionClients().removeAll( this );
    sessionClients().append( this );
}

KSessionManager::~KSessionManager()
{
    sessionClients().removeAll( this );
}

bool KSessionManager::saveState(QSessionManager&)
{
    return true;
}

bool KSessionManager::commitData(QSessionManager&)
{
    return true;
}

QList<KSessionManager *>& KSessionManager::sessionClients()
{
    static QList<KSessionManager*> session_clients;
    return session_clients;
}

void KSessionManager::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }


