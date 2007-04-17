/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2006,2007 Will Stephenson <wstephenson@kde.org>

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
#include "networkmanager_p.h"
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

    class Authentication::Private
    {
    public:
        SecretMap secrets;
    };

    class AuthenticationWep::Private
    {
    public:
        WepMethod method;
        WepType type;
        int keyLength;
    };

    class AuthenticationWpa::Private
    {
    public:
        WpaProtocol protocol;
        WpaVersion version;
        WpaKeyManagement mgmt;
    };

    class AuthenticationWpaEnterprise::Private
    {
    public:
        QString identity;
        QString anonIdentity;
        QString certClient;
        QString certCA;
        QString certPrivate;
        EapMethod method;
        QString idPasswordKey;
        QString certPrivatePasswordKey;
    };
}


/******************************************************************************/

Solid::Authentication::Authentication()
    : d( new Private )
{

}

Solid::Authentication::~Authentication()
{
    delete d;
}

void Solid::Authentication::setSecrets( const SecretMap& secrets )
{
    d->secrets = secrets;
}

Solid::Authentication::SecretMap Solid::Authentication::secrets() const
{
    return d->secrets;
}



/******************************************************************************/

Solid::AuthenticationNone::AuthenticationNone()
{

}

Solid::AuthenticationNone::~AuthenticationNone()
{

}



/******************************************************************************/

Solid::AuthenticationWep::AuthenticationWep()
    : d( new Private )
{

}

Solid::AuthenticationWep::~AuthenticationWep()
{
    delete d;
}

void Solid::AuthenticationWep::setMethod( WepMethod method )
{
    d->method = method;
}

Solid::AuthenticationWep::WepMethod Solid::AuthenticationWep::method() const
{
    return d->method;
}

void Solid::AuthenticationWep::setType( WepType type )
{
    d->type = type;
}

Solid::AuthenticationWep::WepType Solid::AuthenticationWep::type() const
{
    return d->type;
}

void Solid::AuthenticationWep::setKeyLength( int length )
{
    d->keyLength = length;
}

int Solid::AuthenticationWep::keyLength() const
{
    return d->keyLength;
}

/******************************************************************************/

Solid::AuthenticationWpa::AuthenticationWpa()
    : d( new Private )
{

}

Solid::AuthenticationWpa::~AuthenticationWpa()
{
    delete d;
}

void Solid::AuthenticationWpa::setProtocol( WpaProtocol protocol )
{
    d->protocol = protocol;
}

Solid::AuthenticationWpa::WpaProtocol Solid::AuthenticationWpa::protocol() const
{
    return d->protocol;
}

void Solid::AuthenticationWpa::setVersion( WpaVersion version )
{
    d->version = version;
}

Solid::AuthenticationWpa::WpaVersion Solid::AuthenticationWpa::version() const
{
    return d->version;
}

void Solid::AuthenticationWpa::setKeyManagement( WpaKeyManagement mgmt )
{
    d->mgmt = mgmt;
}

Solid::AuthenticationWpa::WpaKeyManagement Solid::AuthenticationWpa::keyManagement() const
{
    return d->mgmt;
}

/******************************************************************************/

Solid::AuthenticationWpaPersonal::AuthenticationWpaPersonal()
{

}

Solid::AuthenticationWpaPersonal::~AuthenticationWpaPersonal()
{

}



/******************************************************************************/

Solid::AuthenticationWpaEnterprise::AuthenticationWpaEnterprise()
    : d( new Private )
{

}

Solid::AuthenticationWpaEnterprise::~AuthenticationWpaEnterprise()
{
    delete d;
}

void Solid::AuthenticationWpaEnterprise::setIdentity( const QString &identity )
{
    d->identity = identity;
}

QString Solid::AuthenticationWpaEnterprise::identity() const
{
    return d->identity;
}

void Solid::AuthenticationWpaEnterprise::setAnonIdentity( const QString &anonIdentity)
{
    d->anonIdentity = anonIdentity;
}

QString Solid::AuthenticationWpaEnterprise::anonIdentity() const
{
    return d->anonIdentity;
}

void Solid::AuthenticationWpaEnterprise::setCertClient( const QString &certClient )
{
    d->certClient = certClient;
}

QString Solid::AuthenticationWpaEnterprise::certClient() const
{
    return d->certClient;
}

void Solid::AuthenticationWpaEnterprise::setCertCA( const QString &certCA )
{
    d->certCA = certCA;
}

QString Solid::AuthenticationWpaEnterprise::certCA() const
{
    return d->certCA;
}

void Solid::AuthenticationWpaEnterprise::setCertPrivate( const QString &certPrivate )
{
    d->certPrivate = certPrivate;
}

QString Solid::AuthenticationWpaEnterprise::certPrivate() const
{
    return d->certPrivate;
}


void Solid::AuthenticationWpaEnterprise::setMethod( EapMethod method )
{
    d->method = method;
}

Solid::AuthenticationWpaEnterprise::EapMethod Solid::AuthenticationWpaEnterprise::method() const
{
    return d->method;
}

void Solid::AuthenticationWpaEnterprise::setIdPasswordKey( const QString &idPasswordKey )
{
    d->idPasswordKey = idPasswordKey;
}

QString Solid::AuthenticationWpaEnterprise::idPasswordKey() const
{
    return d->idPasswordKey;
}

void Solid::AuthenticationWpaEnterprise::setCertPrivatePasswordKey( const QString &certPrivatePasswordKey )
{
    d->certPrivatePasswordKey = certPrivatePasswordKey;
}

QString Solid::AuthenticationWpaEnterprise::certPrivatePasswordKey() const
{
    return d->certPrivatePasswordKey;
}



Solid::AuthenticationValidator::AuthenticationValidator()
    : d( new Private )
{
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( NetworkManager::self().d->backend );

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

bool Solid::AuthenticationValidator::validate( const Solid::Authentication *authentication )
{
    return_SOLID_CALL( Ifaces::AuthenticationValidator*, d->backendObject, false, validate(authentication) );
}

