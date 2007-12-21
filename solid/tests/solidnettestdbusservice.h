/*
   Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
