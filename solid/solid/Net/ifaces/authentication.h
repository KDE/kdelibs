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

#ifndef SOLID_IFACES_AUTHENTICATION_H
#define SOLID_IFACES_AUTHENTICATION_H

#include <QMap>

#include <solid/Net/ifaces/enums.h>

//typedef QValueList<IEEE_802_11_Cipher*> CipherList;
typedef QMap<QString, QString> SecretMap;

namespace Solid
{
namespace Ifaces
{

class Authentication : public Enums::Authentication
{
    public:
        virtual ~Authentication();
        /* hashed credentials, cert passwords, etc */
        virtual void setSecrets( const SecretMap& ) = 0;
        virtual SecretMap getSecrets() const = 0;
        virtual bool isValid( const QString &essid ) = 0;
        virtual void setDefaults() = 0;
};

/* AuthenticationNone */

class AuthenticationNone : public Authentication
{
    public:
        virtual ~AuthenticationNone();
};

/**
 * WEP Authentication
 */
class AuthenticationWEP : public Authentication, public Enums::AuthenticationWEP
{
    public:
        virtual void setMethod( WEPMethod ) = 0;
        virtual WEPMethod getMethod() = 0;
        virtual void setType( WEPType ) = 0;
};

/**
 * AuthenticationWPA contains functionality shared by both Personal and Enterprise
 * authentication flavours
 */
class AuthenticationWPA : public Authentication, public Enums::AuthenticationWPA
{
    public:
        virtual void setProtocol( WPAProtocol ) = 0;
        virtual WPAProtocol getProtocol() = 0;
        virtual void setVersion( WPAVersion ) = 0;
        virtual WPAVersion getVersion() = 0;
};

/**
 * AuthenticationWPAPersonal
 */
class AuthenticationWPAPersonal : public AuthenticationWPA, public Enums::AuthenticationWPAPersonal
{
    public:
        virtual ~AuthenticationWPAPersonal();
};

/**
 * AuthenticationWPAEnterprise
 */
class AuthenticationWPAEnterprise : public AuthenticationWPA, public Enums::AuthenticationWPAEnterprise
{
    public:
        virtual void setIdentity( const QString & ) = 0;
        virtual QString getIdentify() const = 0;
        virtual void setAnonIdentity( const QString & ) = 0;
        virtual QString getAnonIdentity() const = 0;
        virtual void setCertClient( const QString & ) = 0;
        virtual QString getCertClient() const = 0;
        virtual void setCertCA( const QString & ) = 0;
        virtual QString getCertCA() const = 0;
        virtual void setCertPrivate( const QString & ) = 0;
        virtual QString getCertPrivate() const = 0;

        virtual void setMethod( EAPMethod ) = 0;
        virtual EAPMethod getMethod() const = 0;

        virtual QString getIdPasswordKey() const = 0;
        virtual QString getCertPrivatePasswordKey() const = 0;
};

} // Ifaces
} // Solid

#endif /* SOLID_IFACES_AUTHENTICATION_H */
