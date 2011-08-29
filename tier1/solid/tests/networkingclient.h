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

#ifndef KTESTNETWORKSTATUS_H
#define KTESTNETWORKSTATUS_H

#include <kapplication.h>
#include <kmainwindow.h>
//#include <networkstatuscommon.h>

#include "ui_networkingclientview.h"

/**
 * Test client that uses a ConnectionManager to change its state
 *
 * @short Main window class
 * @author Will Stephenson <wstephenson@kde.org>
 * @version 0.1
 */
class TestClient : public KMainWindow
{
Q_OBJECT
public:
    enum AppStatus{ AppDisconnected, AppWaitingForConnect, AppConnected };
    /**
     * Default Constructor
     */
    TestClient();

    /**
     * Default Destructor
     */
    virtual ~TestClient();

private slots:
    void networkStatusChanged( Solid::Networking::Status status );
    void connectButtonClicked();
    void doConnect();
    void doDisconnect();
private:
    void appWaiting();
    void appEstablishing();
    void appIsConnected();
    void appDisestablishing();
    void appDisconnected();
    static QColor toQColor( TestClient::AppStatus );
private:
    //OrgKdeSolidNetworkingClientInterface *m_service;
    Ui_TestClientView ui;
    AppStatus m_status; // this represents the app's status not the network's status
    QWidget * m_view;
};

#endif // KTESTNETWORKSTATUS_H

