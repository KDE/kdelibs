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

#ifndef FAKE_NETWORK_MANAGER_H
#define FAKE_NETWORK_MANAGER_H

#include <QObject>
#include <QStringList>

#include <kdemacros.h>

#include <solid/ifaces/networkmanager.h>
#include <solid/authentication.h>

#include "fakenetworkinterface.h"


class FakeAuthenticationValidator;
class QDomElement;
//class FakeNetworkInterface;

namespace Solid {
    class Authentication;
}

class KDE_EXPORT FakeNetworkManager : public Solid::Ifaces::NetworkManager
{
Q_OBJECT
Q_INTERFACES( Solid::Ifaces::NetworkManager )
    public:
        FakeNetworkManager( QObject * parent, const QStringList & args );
        FakeNetworkManager( QObject * parent, const QStringList & args, const QString & xmlFile );
        virtual ~FakeNetworkManager();
        QStringList networkInterfaces() const;
        QStringList activeNetworkInterfaces() const;
        QObject * createNetworkInterface( const QString &);
        QObject * createAuthenticationValidator();

        bool isNetworkingEnabled() const;
        bool isWirelessEnabled() const;
    public Q_SLOTS:
        void setWirelessEnabled( bool );
        void setNetworkingEnabled( bool );
        void notifyHiddenNetwork( const QString & );

    private:
        void parseNetworkingFile();
        FakeNetworkInterface *parseDeviceElement(const QDomElement &deviceElement);
        QMap<QString,QVariant> parseNetworkElement(const QDomElement &deviceElement);

        bool mUserNetworkingEnabled;
        bool mUserWirelessEnabled;
        QMap<QString, FakeNetworkInterface*> mNetworkInterfaces;
        FakeAuthenticationValidator * mAuthValidator;
        QString mXmlFile;
};

class FakeAuthenticationValidator : public QObject, public Solid::Ifaces::AuthenticationValidator
{
Q_OBJECT
Q_INTERFACES( Solid::Ifaces::AuthenticationValidator )
    public:
        FakeAuthenticationValidator( QObject * parent );
        virtual ~FakeAuthenticationValidator();
        bool validate( const Solid::Authentication * );
};

#endif
