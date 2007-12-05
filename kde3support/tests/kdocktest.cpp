#include "kdocktest.h"

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>

#include <QtGui/QImage>
#include <QtCore/QString>

DockTest::DockTest( QWidget* parent )
  : K3DockArea( parent )
{
  m_blueDock = createDockWidget( "Blue Widget", SmallIcon("internet-mail") );
  //m_blueDock->setDetachable( false );
  m_blueDock->setEnableDocking( K3DockWidget::DockFullSite );
  K3DockWidgetHeader *header = new K3DockWidgetHeader( m_blueDock, "Blue Header" );
  header->forceCloseButtonHidden();
  m_blueDock->setHeader( header );
  m_blueDock->setWindowTitle( "Blue" );
  m_blueDock->setGeometry( 50, 50, 100, 100 );
  QWidget *l = new QWidget( m_blueDock );
  QPalette p1 = l->palette();
  p1.setColor( l->backgroundRole(), Qt::blue );
  l->setPalette( p1 );
  l->setMinimumSize( 100,100 );
  m_blueDock->setWidget( l );

  setMainDockWidget( m_blueDock );

  m_redDock = createDockWidget( "Red Widget", SmallIcon("akregator") );
  m_redDock->setEnableDocking( K3DockWidget::DockFullSite );
  //m_redDock->setDetachable( false );
  header = new K3DockWidgetHeader( m_redDock, "Red kHeader" );
  m_redDock->setHeader( header );
  m_redDock->setWindowTitle( "Red" );
  m_redDock->setGeometry( 50, 50, 100, 100 );
  l = new QWidget( m_redDock );
  QPalette p2 = l->palette();
  p2.setColor( l->backgroundRole(), Qt::red );
  l->setPalette( p2 );
  l->setMinimumSize( 100,100 );
  m_redDock->setWidget( l );
  m_redDock->manualDock( m_blueDock, K3DockWidget::DockLeft, 3000 );

  m_yellowDock = createDockWidget( "Yellow Widget", SmallIcon("internet-web-browser") );
  m_yellowDock->setEnableDocking( K3DockWidget::DockFullSite );
  //m_yellowDock->setDetachable( false );
//  header = new K3DockWidgetHeader( m_yellowDock, "Yellow Header" );
 // m_yellowDock->setHeader( header );
  m_yellowDock->setWindowTitle( "Yellow" );
  m_yellowDock->setGeometry( 50, 50, 100, 100 );
  l = new QWidget( m_yellowDock );
  QPalette p3 = l->palette();
  p3.setColor( l->backgroundRole(), Qt::yellow );
  l->setPalette( p3 );
  l->setMinimumSize( 100,100 );
  m_yellowDock->setWidget( l );
  m_yellowDock->manualDock( m_blueDock, K3DockWidget::DockTop, 5000 );
}

int
main( int argc, char** argv )
{
  KCmdLineArgs::init( argc, argv, "docktest", 0, ki18n("DockTest"), "1.0", ki18n("docktest"));
  KApplication a;
  DockTest* ap = new DockTest();
  ap->setWindowTitle("DockWidget demo");
  ap->show();
  return a.exec();
}

#include "kdocktest.moc"
