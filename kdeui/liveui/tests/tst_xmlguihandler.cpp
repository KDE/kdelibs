#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>

#include "../kliveui.h"
#include "../kliveui_p.h"

int main( int argc, char **argv )
{
  QApplication app( argc, argv );

  QMainWindow window( 0 );
  window.show();

  QAction *action = new QAction( "open", &window );
  action->setObjectName( "open" );
  window.addAction( action );

  action = new QAction( "undo", &window );
  action->setObjectName( "undo" );
  window.addAction( action );

  action = new QAction( "view", &window );
  action->setObjectName( "view" );
  window.addAction( action );

  KLiveUiBuilder builder( &window );
  builder.populateFromXmlGui( "tst_xmlguihandlerui.rc" );

  return app.exec();
}
