/*  This file is part of kdepim.
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <QtGui/QLabel>
#include <QtGui/QPushButton>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>

#include <solid/networking.h>
//#include <networkstatusindicator.h>

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
    : KMainWindow( 0 ),
      m_status( AppDisconnected ), m_view( new QWidget( this ) )
{
    ui.setupUi( m_view );
    //new StatusBarNetworkStatusIndicator( m_view );

    setCentralWidget(m_view);

    networkStatusChanged( Solid::Networking::status() );
    appDisconnected();

    kDebug() << "About to connect" << endl;
    connect( Solid::Networking::notifier(), SIGNAL( statusChanged( Solid::Networking::Status ) ), SLOT( networkStatusChanged( Solid::Networking::Status ) ) );
    kDebug() << "Connected." << endl;
    connect( Solid::Networking::notifier(), SIGNAL( shouldConnect() ), this, SLOT( doConnect() ) );
    connect( Solid::Networking::notifier(), SIGNAL( shouldDisconnect() ), this, SLOT( doDisconnect() ) );

    connect( ui.connectButton, SIGNAL( clicked() ), SLOT( connectButtonClicked() ) );
}

TestClient::~TestClient()
{
}

void TestClient::networkStatusChanged( Solid::Networking::Status status )
{
    kDebug() << k_funcinfo << endl;
    kDebug() << "Networking is now: " << toString( status ) << " (" << status << ")" << endl;
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
  kDebug() << k_funcinfo << endl;
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
  kDebug() << k_funcinfo << endl;
  //m_status = AppWaitingForConnect;
  ui.appStatusLabel->setText( "Waiting" );
}

void TestClient::appIsConnected()
{
  kDebug() << k_funcinfo << endl;
  ui.connectButton->setEnabled( true );
  ui.connectButton->setText( "Disconnect" );
  ui.appStatusLabel->setText( "Connected" );
  m_status = AppConnected;
}

void TestClient::appEstablishing()
{
  kDebug() << k_funcinfo << endl;
  ui.netStatusLabel->setText( "Establishing" );
  ui.connectButton->setEnabled( false );
}

void TestClient::appDisestablishing( )
{
  kDebug() << k_funcinfo << endl;
  ui.connectButton->setEnabled( false );
  ui.appStatusLabel->setText( "Disconnected" );
}

void TestClient::appDisconnected( )
{
  kDebug() << k_funcinfo << endl;
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
static const char description[] =
    I18N_NOOP("Test Client for Network Status kded module");

static const char version[] = "v0.1";

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
  KAboutData about("KNetworkStatusTestClient", I18N_NOOP("knetworkstatustestclient"), version, description, KAboutData::License_GPL, "(C) 2007 Will Stephenson", 0, 0, "wstephenson@kde.org");
  about.addAuthor( "Will Stephenson", 0, "wstephenson@kde.org" );
  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions(options);
  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count() == 0)
  {
    TestClient *widget = new TestClient;
    widget->show();
  }
  else
  {
    int i = 0;
    for (; i < args->count(); i++)
    {
      TestClient *widget = new TestClient;
      widget->show();
    }
  }
  args->clear();

  return app.exec();
}

#include "networkingclient.moc"

