#include "kdocktest.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>

#include <qwidget.h>
#include <qstring.h>

DockTest::DockTest( QWidget* parent )
  : K3DockArea( parent )
{
  m_blueDock = createDockWidget( "Blue Widget", SmallIcon("mail") );
  //m_blueDock->setDetachable( false );
  m_blueDock->setEnableDocking( K3DockWidget::DockFullSite );
  K3DockWidgetHeader *header = new K3DockWidgetHeader( m_blueDock, "Blue Header" );
  header->forceCloseButtonHidden();
  m_blueDock->setHeader( header );
  m_blueDock->setCaption( "Blue" );
  m_blueDock->setGeometry( 50, 50, 100, 100 );
  QWidget *l = new QWidget( m_blueDock );
  l->setBackgroundColor( Qt::blue );
  l->setMinimumSize( 100,100 );
  m_blueDock->setWidget( l );

  setMainDockWidget( m_blueDock );

  m_redDock = createDockWidget( "Red Widget", SmallIcon("news") );
  m_redDock->setEnableDocking( K3DockWidget::DockFullSite );
  //m_redDock->setDetachable( false );
  header = new K3DockWidgetHeader( m_redDock, "Red kHeader" );
  m_redDock->setHeader( header );
  m_redDock->setCaption( "Red" );
  m_redDock->setGeometry( 50, 50, 100, 100 );
  l = new QWidget( m_redDock );
  l->setBackgroundColor( Qt::red );
  l->setMinimumSize( 100,100 );
  m_redDock->setWidget( l );
  m_redDock->manualDock( m_blueDock, K3DockWidget::DockLeft, 3000 );

  m_yellowDock = createDockWidget( "Yellow Widget", SmallIcon("web") );
  m_yellowDock->setEnableDocking( K3DockWidget::DockFullSite );
  //m_yellowDock->setDetachable( false );
//  header = new K3DockWidgetHeader( m_yellowDock, "Yellow Header" );
 // m_yellowDock->setHeader( header );
  m_yellowDock->setCaption( "Yellow" );
  m_yellowDock->setGeometry( 50, 50, 100, 100 );
  l = new QWidget( m_yellowDock );
  l->setBackgroundColor( Qt::yellow );
  l->setMinimumSize( 100,100 );
  m_yellowDock->setWidget( l );
  m_yellowDock->manualDock( m_blueDock, K3DockWidget::DockTop, 5000 );
}

int
main( int argc, char** argv )
{
  KCmdLineArgs::init( argc, argv, "docktest", "DockTest", "docktest", "1.0" );
  KApplication a;
  DockTest* ap = new DockTest();
  ap->setCaption("DockWidget demo");
  a.setMainWidget( ap );
  ap->show();
  return a.exec();
}

#include "kdocktest.moc"
