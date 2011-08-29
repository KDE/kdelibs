/*
    Copyright 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SOLID_NETWORKING_TESTSERVICE_H
#define SOLID_NETWORKING_TESTSERVICE_H

//#include <solid/networking.h>
#include <QtCore/QObject>

class QTimer;

class TestNetworkingService;

class Behaviour : public QObject
{
    Q_OBJECT
    public:
        Behaviour( TestNetworkingService * );
    public Q_SLOTS:
        virtual void go() = 0;
        virtual void serviceStatusChanged( uint ) = 0;
    protected:
        TestNetworkingService * mService;
};

class GoOnlineOnRequest : public Behaviour
{
Q_OBJECT
public:
    GoOnlineOnRequest( TestNetworkingService * );
public Q_SLOTS:
    void go();
    void serviceStatusChanged( uint );
private Q_SLOTS:
    void doDelayedConnect();
    void doDelayedDisconnect();
};

class TestNetworkingService : public QObject
{
Q_OBJECT

    Q_PROPERTY( uint Status  READ status )
public:
    TestNetworkingService( const QString & behaviour );
    ~TestNetworkingService();
    void setStatus( uint );
public Q_SLOTS:
    uint requestConnection(); /*Result*/
    void releaseConnection();
    uint status() const;
Q_SIGNALS:
    void statusChanged( uint );
private:
    uint mStatus;
    Behaviour * mBehaviour;
};

#endif
