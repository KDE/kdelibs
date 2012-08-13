
#include "kmainwindowrestoretest.h"

#include <QApplication>
#include <QLabel>

#define MAKE_WINDOW( kind, title ) do { \
  MainWin##kind * m = new MainWin##kind; \
  m->setCaption( title ); \
  m->setCentralWidget( new QLabel( title, m ) ); \
  m->show(); \
} while ( false )

int main( int argc, char * argv[] ) {

  QApplication::setApplicationName("kmainwindowrestoretest");
  QApplication app(argc, argv);

  if ( qApp->isSessionRestored() ) {
    kRestoreMainWindows< MainWin1, MainWin2, MainWin3 >();
    kRestoreMainWindows< MainWin4, MainWin5 >();
    RESTORE(MainWin6);
    //kRestoreMainWindows< MainWin6 >(); // should be equivalent to RESTORE()
  } else {
    MAKE_WINDOW( 1, "First 1" );
    MAKE_WINDOW( 1, "Second 1" );
    MAKE_WINDOW( 2, "Only 2" );
    MAKE_WINDOW( 3, "First 3" );
    MAKE_WINDOW( 4, "First 4" );
    MAKE_WINDOW( 4, "Second 4" );
    MAKE_WINDOW( 3, "Second 3" );
    MAKE_WINDOW( 4, "Third 4" );
    MAKE_WINDOW( 5, "First 5" );
    MAKE_WINDOW( 5, "Second 5" );
    MAKE_WINDOW( 1, "Only 6" );
  }

  return app.exec();
}

