/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

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
#include "authentication.h"
#include <solid/networkmanager.h>
#include <solid/ifaces/networkmanager.h>
#include <solid/ifaces/authentication.h>

namespace Solid
{
    class AuthenticationValidator::Private
    {
    public:
        QObject *backendObject;
    };
}

Solid::AuthenticationValidator::AuthenticationValidator()
    : d( new Private )
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( NetworkManager::self().managerBackend() );

    if ( backend )
    {
        d->backendObject = backend->createAuthenticationValidator();
    }
}

Solid::AuthenticationValidator::~AuthenticationValidator()
{
    delete d->backendObject;
    delete d;
}

bool Solid::AuthenticationValidator::validate( const Ifaces::Authentication *authentication )
{
    return_SOLID_CALL( Ifaces::AuthenticationValidator*, d->backendObject, false, validate(authentication) );
}

