/*  This file is part of the KDE project
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

#include "authentication.h"


namespace Solid
{
namespace Ifaces
{
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
    };

    class AuthenticationWpa::Private
    {
    public:
        WpaProtocol protocol;
        WpaVersion version;
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
}



/******************************************************************************/

Solid::Ifaces::Authentication::Authentication()
    : d( new Private )
{

}

Solid::Ifaces::Authentication::~Authentication()
{
    delete d;
}

void Solid::Ifaces::Authentication::setSecrets( const SecretMap& secrets )
{
    d->secrets = secrets;
}

Solid::Ifaces::Authentication::SecretMap Solid::Ifaces::Authentication::secrets() const
{
    return d->secrets;
}



/******************************************************************************/

Solid::Ifaces::AuthenticationNone::AuthenticationNone()
{

}

Solid::Ifaces::AuthenticationNone::~AuthenticationNone()
{

}



/******************************************************************************/

Solid::Ifaces::AuthenticationWep::AuthenticationWep()
    : d( new Private )
{

}

Solid::Ifaces::AuthenticationWep::~AuthenticationWep()
{
    delete d;
}

void Solid::Ifaces::AuthenticationWep::setMethod( WepMethod method )
{
    d->method = method;
}

Solid::Ifaces::AuthenticationWep::WepMethod Solid::Ifaces::AuthenticationWep::method() const
{
    return d->method;
}

void Solid::Ifaces::AuthenticationWep::setType( WepType type )
{
    d->type = type;
}

Solid::Ifaces::AuthenticationWep::WepType Solid::Ifaces::AuthenticationWep::type() const
{
    return d->type;
}



/******************************************************************************/

Solid::Ifaces::AuthenticationWpa::AuthenticationWpa()
    : d( new Private )
{

}

Solid::Ifaces::AuthenticationWpa::~AuthenticationWpa()
{
    delete d;
}

void Solid::Ifaces::AuthenticationWpa::setProtocol( WpaProtocol protocol )
{
    d->protocol = protocol;
}

Solid::Ifaces::AuthenticationWpa::WpaProtocol Solid::Ifaces::AuthenticationWpa::protocol() const
{
    return d->protocol;
}

void Solid::Ifaces::AuthenticationWpa::setVersion( WpaVersion version )
{
    d->version = version;
}

Solid::Ifaces::AuthenticationWpa::WpaVersion Solid::Ifaces::AuthenticationWpa::version() const
{
    return d->version;
}



/******************************************************************************/

Solid::Ifaces::AuthenticationWpaPersonal::AuthenticationWpaPersonal()
{

}

Solid::Ifaces::AuthenticationWpaPersonal::~AuthenticationWpaPersonal()
{

}



/******************************************************************************/

Solid::Ifaces::AuthenticationWpaEnterprise::AuthenticationWpaEnterprise()
    : d( new Private )
{

}

Solid::Ifaces::AuthenticationWpaEnterprise::~AuthenticationWpaEnterprise()
{
    delete d;
}

void Solid::Ifaces::AuthenticationWpaEnterprise::setIdentity( const QString &identity )
{
    d->identity = identity;
}

QString Solid::Ifaces::AuthenticationWpaEnterprise::identity() const
{
    return d->identity;
}

void Solid::Ifaces::AuthenticationWpaEnterprise::setAnonIdentity( const QString &anonIdentity)
{
    d->anonIdentity = anonIdentity;
}

QString Solid::Ifaces::AuthenticationWpaEnterprise::anonIdentity() const
{
    return d->anonIdentity;
}

void Solid::Ifaces::AuthenticationWpaEnterprise::setCertClient( const QString &certClient )
{
    d->certClient = certClient;
}

QString Solid::Ifaces::AuthenticationWpaEnterprise::certClient() const
{
    return d->certClient;
}

void Solid::Ifaces::AuthenticationWpaEnterprise::setCertCA( const QString &certCA )
{
    d->certCA = certCA;
}

QString Solid::Ifaces::AuthenticationWpaEnterprise::certCA() const
{
    return d->certCA;
}

void Solid::Ifaces::AuthenticationWpaEnterprise::setCertPrivate( const QString &certPrivate )
{
    d->certPrivate = certPrivate;
}

QString Solid::Ifaces::AuthenticationWpaEnterprise::certPrivate() const
{
    return d->certPrivate;
}


void Solid::Ifaces::AuthenticationWpaEnterprise::setMethod( EapMethod method )
{
    d->method = method;
}

Solid::Ifaces::AuthenticationWpaEnterprise::EapMethod Solid::Ifaces::AuthenticationWpaEnterprise::method() const
{
    return d->method;
}

void Solid::Ifaces::AuthenticationWpaEnterprise::setIdPasswordKey( const QString &idPasswordKey )
{
    d->idPasswordKey = idPasswordKey;
}

QString Solid::Ifaces::AuthenticationWpaEnterprise::idPasswordKey() const
{
    return d->idPasswordKey;
}

void Solid::Ifaces::AuthenticationWpaEnterprise::setCertPrivatePasswordKey( const QString &certPrivatePasswordKey )
{
    d->certPrivatePasswordKey = certPrivatePasswordKey;
}

QString Solid::Ifaces::AuthenticationWpaEnterprise::certPrivatePasswordKey() const
{
    return d->certPrivatePasswordKey;
}


/******************************************************************************/

Solid::Ifaces::AuthenticationValidator::~AuthenticationValidator()
{
}
