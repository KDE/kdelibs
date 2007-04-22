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

#include "../soliddefs_p.h"
#include "authentication.h"
#include "networkmanager_p.h"
#include <solid/experimental/networkmanager.h>
#include <solid/experimental/ifaces/networkmanager.h>
#include <solid/experimental/ifaces/authentication.h>

namespace SolidExperimental
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

SolidExperimental::Authentication::Authentication()
    : d( new Private )
{

}

SolidExperimental::Authentication::~Authentication()
{
    delete d;
}

void SolidExperimental::Authentication::setSecrets( const SecretMap& secrets )
{
    d->secrets = secrets;
}

SolidExperimental::Authentication::SecretMap SolidExperimental::Authentication::secrets() const
{
    return d->secrets;
}



/******************************************************************************/

SolidExperimental::AuthenticationNone::AuthenticationNone()
{

}

SolidExperimental::AuthenticationNone::~AuthenticationNone()
{

}



/******************************************************************************/

SolidExperimental::AuthenticationWep::AuthenticationWep()
    : d( new Private )
{

}

SolidExperimental::AuthenticationWep::~AuthenticationWep()
{
    delete d;
}

void SolidExperimental::AuthenticationWep::setMethod( WepMethod method )
{
    d->method = method;
}

SolidExperimental::AuthenticationWep::WepMethod SolidExperimental::AuthenticationWep::method() const
{
    return d->method;
}

void SolidExperimental::AuthenticationWep::setType( WepType type )
{
    d->type = type;
}

SolidExperimental::AuthenticationWep::WepType SolidExperimental::AuthenticationWep::type() const
{
    return d->type;
}

void SolidExperimental::AuthenticationWep::setKeyLength( int length )
{
    d->keyLength = length;
}

int SolidExperimental::AuthenticationWep::keyLength() const
{
    return d->keyLength;
}

/******************************************************************************/

SolidExperimental::AuthenticationWpa::AuthenticationWpa()
    : d( new Private )
{

}

SolidExperimental::AuthenticationWpa::~AuthenticationWpa()
{
    delete d;
}

void SolidExperimental::AuthenticationWpa::setProtocol( WpaProtocol protocol )
{
    d->protocol = protocol;
}

SolidExperimental::AuthenticationWpa::WpaProtocol SolidExperimental::AuthenticationWpa::protocol() const
{
    return d->protocol;
}

void SolidExperimental::AuthenticationWpa::setVersion( WpaVersion version )
{
    d->version = version;
}

SolidExperimental::AuthenticationWpa::WpaVersion SolidExperimental::AuthenticationWpa::version() const
{
    return d->version;
}

void SolidExperimental::AuthenticationWpa::setKeyManagement( WpaKeyManagement mgmt )
{
    d->mgmt = mgmt;
}

SolidExperimental::AuthenticationWpa::WpaKeyManagement SolidExperimental::AuthenticationWpa::keyManagement() const
{
    return d->mgmt;
}

/******************************************************************************/

SolidExperimental::AuthenticationWpaPersonal::AuthenticationWpaPersonal()
{

}

SolidExperimental::AuthenticationWpaPersonal::~AuthenticationWpaPersonal()
{

}



/******************************************************************************/

SolidExperimental::AuthenticationWpaEnterprise::AuthenticationWpaEnterprise()
    : d( new Private )
{

}

SolidExperimental::AuthenticationWpaEnterprise::~AuthenticationWpaEnterprise()
{
    delete d;
}

void SolidExperimental::AuthenticationWpaEnterprise::setIdentity( const QString &identity )
{
    d->identity = identity;
}

QString SolidExperimental::AuthenticationWpaEnterprise::identity() const
{
    return d->identity;
}

void SolidExperimental::AuthenticationWpaEnterprise::setAnonIdentity( const QString &anonIdentity)
{
    d->anonIdentity = anonIdentity;
}

QString SolidExperimental::AuthenticationWpaEnterprise::anonIdentity() const
{
    return d->anonIdentity;
}

void SolidExperimental::AuthenticationWpaEnterprise::setCertClient( const QString &certClient )
{
    d->certClient = certClient;
}

QString SolidExperimental::AuthenticationWpaEnterprise::certClient() const
{
    return d->certClient;
}

void SolidExperimental::AuthenticationWpaEnterprise::setCertCA( const QString &certCA )
{
    d->certCA = certCA;
}

QString SolidExperimental::AuthenticationWpaEnterprise::certCA() const
{
    return d->certCA;
}

void SolidExperimental::AuthenticationWpaEnterprise::setCertPrivate( const QString &certPrivate )
{
    d->certPrivate = certPrivate;
}

QString SolidExperimental::AuthenticationWpaEnterprise::certPrivate() const
{
    return d->certPrivate;
}


void SolidExperimental::AuthenticationWpaEnterprise::setMethod( EapMethod method )
{
    d->method = method;
}

SolidExperimental::AuthenticationWpaEnterprise::EapMethod SolidExperimental::AuthenticationWpaEnterprise::method() const
{
    return d->method;
}

void SolidExperimental::AuthenticationWpaEnterprise::setIdPasswordKey( const QString &idPasswordKey )
{
    d->idPasswordKey = idPasswordKey;
}

QString SolidExperimental::AuthenticationWpaEnterprise::idPasswordKey() const
{
    return d->idPasswordKey;
}

void SolidExperimental::AuthenticationWpaEnterprise::setCertPrivatePasswordKey( const QString &certPrivatePasswordKey )
{
    d->certPrivatePasswordKey = certPrivatePasswordKey;
}

QString SolidExperimental::AuthenticationWpaEnterprise::certPrivatePasswordKey() const
{
    return d->certPrivatePasswordKey;
}



SolidExperimental::AuthenticationValidator::AuthenticationValidator()
    : d( new Private )
{
#if 0
    Ifaces::NetworkManager *backend = qobject_cast<Ifaces::NetworkManager*>( NetworkManager::self().d->managerBackend() );

    if ( backend )
    {
        d->backendObject = backend->createAuthenticationValidator();
    }
#endif
}

SolidExperimental::AuthenticationValidator::~AuthenticationValidator()
{
    delete d->backendObject;
    delete d;
}

bool SolidExperimental::AuthenticationValidator::validate( const SolidExperimental::Authentication *authentication )
{
    return_SOLID_CALL( Ifaces::AuthenticationValidator*, d->backendObject, false, validate(authentication) );
}

