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
#include <QString>

namespace Solid
{
namespace Ifaces
{

    class Authentication
    {
    public:
        //typedef QValueList<IEEE_802_11_Cipher*> CipherList;
        typedef QMap<QString, QString> SecretMap;

        Authentication();
        virtual ~Authentication();

        /* hashed credentials, cert passwords, etc */
        void setSecrets( const SecretMap& );
        SecretMap secrets() const;

        virtual bool isValid( const QString &essid ) const = 0;

    private:
        class Private;
        Private *d;
    };

    class AuthenticationNone : public Authentication
    {
    public:
        AuthenticationNone();
        virtual ~AuthenticationNone();

        virtual bool isValid( const QString &essid ) const;
    };

    /**
     * WEP Authentication
     */
    class AuthenticationWep : public Authentication
    {
    public:
        enum WepType { WepAscii, WepHex, WepPassphrase };
        enum WepMethod { WepOpenSystem, WepSharedKey };

        AuthenticationWep();
        virtual ~AuthenticationWep();

        virtual bool isValid( const QString &essid ) const;

        void setMethod( WepMethod );
        WepMethod method() const;

        void setType( WepType );
        WepType type() const;

    private:
        class Private;
        Private *d;
    };

    /**
     * AuthenticationWpa contains functionality shared by both Personal and Enterprise
     * authentication flavours
     */
    class AuthenticationWpa : public Authentication
    {
    public:
        enum WpaProtocol { WpaAuto, WpaTkip, WpaCcmpAes, // WPA Personal only
                           WpaEap /* WPA Enterprise only */ };
        enum WpaVersion { Wpa1, Wpa2 };

        AuthenticationWpa();
        virtual ~AuthenticationWpa();

        void setProtocol( WpaProtocol );
        WpaProtocol protocol() const;

        void setVersion( WpaVersion );
        WpaVersion version() const;

    private:
        class Private;
        Private *d;
    };

    /**
     * AuthenticationWpaPersonal
     */
    class AuthenticationWpaPersonal : public AuthenticationWpa
    {
    public:
        AuthenticationWpaPersonal();
        virtual ~AuthenticationWpaPersonal();

        virtual bool isValid( const QString &essid ) const;
    };

    /**
     * AuthenticationWpaEnterprise
     */
    class AuthenticationWpaEnterprise : public AuthenticationWpa
    {
    public:
        enum EapMethod { EapPeap, EapTls, EapTtls };

        AuthenticationWpaEnterprise();
        virtual ~AuthenticationWpaEnterprise();

        virtual bool isValid( const QString &essid ) const;

        void setIdentity( const QString & );
        QString identity() const;

        void setAnonIdentity( const QString & );
        QString anonIdentity() const;

        void setCertClient( const QString & );
        QString certClient() const;

        void setCertCA( const QString & );
        QString certCA() const;

        void setCertPrivate( const QString & );
        QString certPrivate() const;

        void setMethod( EapMethod );
        EapMethod method() const;

        void setIdPasswordKey( const QString & );
        QString idPasswordKey() const;

        void setCertPrivatePasswordKey( const QString & );
        QString certPrivatePasswordKey() const;

    private:
        class Private;
        Private *d;
    };

} // Ifaces
} // Solid

#endif /* SOLID_IFACES_AUTHENTICATION_H */
