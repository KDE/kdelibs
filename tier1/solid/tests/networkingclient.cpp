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

#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <qdebug.h>

#include "networkingclient.h"

QString toString( Solid::Networking::Status st )
{
  QString str;
  switch ( st ) {
    case Solid::Networking::Unknown:
      str = "Unknown";
      break;
    case Solid::Networking::Unconnected:
      str = "Unconnected";
      break;
    case Solid::Networking::Disconnecting:
      str = "Disconnecting";
      break;
    case Solid::Networking::Connecting:
      str = "Connecting";
      break;
    case Solid::Networking::Connected:
      str = "Connected";
      break;
  }
  return str;
}

TestClient::TestClient()
    : QMainWindow( 0 ),
      m_status( AppDisconnected ), m_view( new QWidget( this ) )
{
    ui.setupUi( m_view );

    setCentralWidget(m_view);

    networkStatusChanged( Solid::Networking::status() );
    appDisconnected();

    qDebug() << "About to connect";
    connect( Solid::Networking::notifier(), SIGNAL( statusChanged( Solid::Networking::Status ) ), SLOT( networkStatusChanged( Solid::Networking::Status ) ) );
    qDebug() << "Connected.";
    connect( Solid::Networking::notifier(), SIGNAL( shouldConnect() ), this, SLOT( doConnect() ) );
    connect( Solid::Networking::notifier(), SIGNAL( shouldDisconnect() ), this, SLOT( doDisconnect() ) );

    connect( ui.connectButton, SIGNAL( clicked() ), SLOT( connectButtonClicked() ) );
}

TestClient::~TestClient()
{
}

void TestClient::networkStatusChanged( Solid::Networking::Status status )
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << "Networking is now: " << toString( status ) << " (" << status << ")";
    ui.netStatusLabel->setText( toString( status ) );
    QPalette palette;
    palette.setColor( ui.netStatusLabel->backgroundRole(), toQColor( m_status ) );
    ui.netStatusLabel->setPalette( palette );
}

void TestClient::doConnect()
{
  Q_ASSERT( Solid::Networking::status() == Solid::Networking::Connected );
  if ( m_status != AppConnected ) {
    appIsConnected();
  }
}

void TestClient::doDisconnect()
{
  Q_ASSERT( Solid::Networking::status() != Solid::Networking::Connected );
  if ( m_status == AppConnected ) {
    appDisconnected();
  }
}

void TestClient::connectButtonClicked()
{
  qDebug() << Q_FUNC_INFO;
  if ( m_status == AppDisconnected ) {
    switch ( Solid::Networking::status() )
    {
      case Solid::Networking::Unknown:
      case Solid::Networking::Connected:
        appIsConnected();
        break;
      default:
        appWaiting();
        break;
    }
  }
  else if ( m_status == AppConnected || m_status == AppWaitingForConnect ) {
    appDisconnected();
  }
}

void TestClient::appWaiting()
{
  qDebug() << Q_FUNC_INFO;
  //m_status = AppWaitingForConnect;
  ui.appStatusLabel->setText( "Waiting" );
}

void TestClient::appIsConnected()
{
  qDebug() << Q_FUNC_INFO;
  ui.connectButton->setEnabled( true );
  ui.connectButton->setText( "Disconnect" );
  ui.appStatusLabel->setText( "Connected" );
  m_status = AppConnected;
}

void TestClient::appEstablishing()
{
  qDebug() << Q_FUNC_INFO;
  ui.netStatusLabel->setText( "Establishing" );
  ui.connectButton->setEnabled( false );
}

void TestClient::appDisestablishing( )
{
  qDebug() << Q_FUNC_INFO;
  ui.connectButton->setEnabled( false );
  ui.appStatusLabel->setText( "Disconnected" );
}

void TestClient::appDisconnected( )
{
  qDebug() << Q_FUNC_INFO;
  ui.connectButton->setEnabled( true );
  ui.connectButton->setText( "Start Connect" );
  ui.appStatusLabel->setText( "Disconnected" );
  m_status = AppDisconnected;
}

QColor TestClient::toQColor( TestClient::AppStatus st )
{
    QColor col;
    switch ( st ) {
      case TestClient::AppDisconnected:
        col = Qt::red;
        break;
      case TestClient::AppWaitingForConnect:
        col = Qt::yellow;
        break;
      case TestClient::AppConnected:
        col = Qt::green;
        break;
    }
    return col;
}

//main
int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  if (app.arguments().count() == 0)
  {
    TestClient *widget = new TestClient;
    widget->show();
  }
  else
  {
    int i = 0;
    for (; i < app.arguments().count(); i++)
    {
      TestClient *widget = new TestClient;
      widget->show();
    }
  }
  app.arguments().clear();

  return app.exec();
}

#include "networkingclient.moc"

