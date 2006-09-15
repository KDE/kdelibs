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

#ifndef SOLID_NET_AUTHENTICATION_H
#define SOLID_NET_AUTHENTICATION_H

#include <solid/Net/ifaces/authentication.h>
#include <solid/Net/ifaces/enums.h>
#include <QObject>

namespace Solid
{
namespace Net
{

/**
 * Abstract base Authentication
 */
class Authentication : public QObject
{
    public:
        Authentication();
        ~Authentication();
        /* hashed credentials, cert passwords, etc */
        virtual void setSecrets( const SecretMap& );
        virtual SecretMap secrets() const;
    protected:
        class Private;
        Private * d;
};

/**
 * AuthenticationNone - an unencrypted network
 */
class AuthenticationNone : public Authentication
{
    public:
        virtual bool isValid( const QString &essid );
        virtual void setDefaults();
};

/**
 * WEP Authentication
 */
class AuthenticationWEP : public Authentication
{
    public:
        AuthenticationWEP( Ifaces::AuthenticationWEP::WEPMethod method );
        virtual ~AuthenticationWEP();
        virtual void setMethod( Ifaces::AuthenticationWEP::WEPMethod );
        virtual Ifaces::AuthenticationWEP::WEPMethod method();
        virtual void setType( Ifaces::AuthenticationWEP::WEPType );
        virtual Ifaces::AuthenticationWEP::WEPType type();
    protected:
        class Private;
        Private * d;
};

/**
 * AuthenticationWPA contains functionality shared by both Personal and Enterprise
 * authentication flavours
 */
class AuthenticationWPA : public Authentication
{
    public:
        AuthenticationWPA();
        virtual ~AuthenticationWPA();

        virtual void setProtocol( Ifaces::AuthenticationWPA::WPAProtocol );
        virtual Ifaces::AuthenticationWPA::WPAProtocol protocol();
        virtual void setVersion( Ifaces::AuthenticationWPA::WPAVersion );
        virtual Ifaces::AuthenticationWPA::WPAVersion version();
    protected:
        class Private;
        Private * d;
};

/**
 * AuthenticationWPAPersonal
 */
class AuthenticationWPAPersonal : public AuthenticationWPA
{

};

/**
 * AuthenticationWPAEnterprise
 */
class AuthenticationWPAEnterprise : public AuthenticationWPA
{
    public:
        AuthenticationWPAEnterprise();
        virtual ~AuthenticationWPAEnterprise();

        virtual void setIdentity( const QString & );
        virtual QString identify() const;
        virtual void setAnonIdentity( const QString & );
        virtual QString anonIdentity() const;
        virtual void setCertClient( const QString & );
        virtual QString certClient() const;
        virtual void setCertCA( const QString & );
        virtual QString certCA() const;
        virtual void setCertPrivate( const QString & );
        virtual QString certPrivate() const;

        virtual void setMethod( Ifaces::AuthenticationWPAEnterprise::EAPMethod );
        virtual Ifaces::AuthenticationWPAEnterprise::EAPMethod method() const;

        virtual QString idPasswordKey() const;
        virtual QString certPrivatePasswordKey() const;
    private:
        class Private;
        Private * d;
};

} // Net
} // Solid

#endif /* SOLID_NET_AUTHENTICATION_H */
