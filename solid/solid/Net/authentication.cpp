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
    class Authentication::Private
    {
        public:
            SecretMap secrets;
    };
    class AuthenticationWEP::Private
    {
        public:
            Ifaces::AuthenticationWEP::WEPMethod method;
            Ifaces::AuthenticationWEP::WEPType type;
    };
    class AuthenticationWPA::Private
    {
        public:
            Ifaces::AuthenticationWPA::WPAProtocol protocol;
            Ifaces::AuthenticationWPA::WPAVersion version;
    };
    class AuthenticationWPAEnterprise::Private
    {
        public:
            QString identity;
            QString anonIdentity;
            QString certClient;
            QString certCA;
            QString certPrivate;
            Ifaces::AuthenticationWPAEnterprise::EAPMethod method;
            QString idPasswordKey;
            QString certPrivatePasswordKey;
    };
}

Solid::Authentication::Authentication()
{
    d = new Private();
}

Solid::Authentication::~Authentication()
{
    delete d;
}

void Solid::Authentication::setSecrets( const SecretMap& secrets )
{
    d->secrets = secrets;
}

SecretMap Solid::Authentication::secrets() const
{
    return d->secrets;
}

bool Solid::AuthenticationNone::isValid( const QString &essid )
{
    return true;
}

void Solid::AuthenticationNone::setDefaults()
{
    return;
}

Solid::AuthenticationWEP::AuthenticationWEP( Ifaces::AuthenticationWEP::WEPMethod method )
{
    d = new Private;
    d->method = method;
}

Solid::AuthenticationWEP::~AuthenticationWEP()
{
    delete d;
}

void Solid::AuthenticationWEP::setMethod( Ifaces::AuthenticationWEP::WEPMethod method )
{
    d->method = method;
}

Solid::Ifaces::AuthenticationWEP::WEPMethod Solid::AuthenticationWEP::method()
{
    return d->method;
}

void Solid::AuthenticationWEP::setType( Ifaces::AuthenticationWEP::WEPType type )
{
    d->type = type;
}

Solid::Ifaces::AuthenticationWEP::WEPType Solid::AuthenticationWEP::type()
{
    return d->type;
}

Solid::AuthenticationWPA::AuthenticationWPA()
{
    d = new Private;
}

Solid::AuthenticationWPA::~AuthenticationWPA()
{
    delete d;
}

void Solid::AuthenticationWPA::setProtocol( Ifaces::AuthenticationWPA::WPAProtocol protocol )
{
    d->protocol = protocol;
}

Solid::Ifaces::AuthenticationWPA::WPAProtocol Solid::AuthenticationWPA::protocol()
{
    return d->protocol;
}

void Solid::AuthenticationWPA::setVersion( Ifaces::AuthenticationWPA::WPAVersion version )
{
    d->version = version;
}

Solid::Ifaces::AuthenticationWPA::WPAVersion Solid::AuthenticationWPA::version()
{
    return d->version;
}

Solid::AuthenticationWPAEnterprise::AuthenticationWPAEnterprise()
{
    d = new Private;
}

Solid::AuthenticationWPAEnterprise::~AuthenticationWPAEnterprise()
{
    delete d;
}

void Solid::AuthenticationWPAEnterprise::setIdentity( const QString & identity )
{
    d->identity = identity;
}

QString Solid::AuthenticationWPAEnterprise::identify() const
{
    return d->identity;
}

void Solid::AuthenticationWPAEnterprise::setAnonIdentity( const QString & anonIdentity)
{
    d->anonIdentity = anonIdentity;
}

QString Solid::AuthenticationWPAEnterprise::anonIdentity() const
{
    return d->anonIdentity;
}

void Solid::AuthenticationWPAEnterprise::setCertClient( const QString & certClient )
{
    d->certClient = certClient;
}

QString Solid::AuthenticationWPAEnterprise::certClient() const
{
    return d->certClient;
}

void Solid::AuthenticationWPAEnterprise::setCertCA( const QString & certCA)
{
    d->certCA = certCA;
}

QString Solid::AuthenticationWPAEnterprise::certCA() const
{
    return d->certCA;
}

void Solid::AuthenticationWPAEnterprise::setCertPrivate( const QString & certPrivate)
{
    d->certPrivate = certPrivate;
}

QString Solid::AuthenticationWPAEnterprise::certPrivate() const
{
    return d->certPrivate;
}


void Solid::AuthenticationWPAEnterprise::setMethod( Ifaces::AuthenticationWPAEnterprise::EAPMethod method )
{
    d->method = method;
}

Solid::Ifaces::AuthenticationWPAEnterprise::EAPMethod Solid::AuthenticationWPAEnterprise::method() const
{
    return d->method;
}


QString Solid::AuthenticationWPAEnterprise::idPasswordKey() const
{
    return d->idPasswordKey;
}

QString Solid::AuthenticationWPAEnterprise::certPrivatePasswordKey() const
{
    return d->certPrivatePasswordKey;
}


