#include <kapplication.h>
#include <keditcl.h>
#include <qpopupmenu.h>

int main( int argc, char **argv )
{
  KApplication app( argc, argv, "kedittest" );
  KEdit *edit = new KEdit( 0L );
  QPopupMenu *pop = new QPopupMenu( 0L );
  pop->insertItem( "Popupmenu item" );
  edit->installRBPopup( pop );
  edit->show();
  return app.exec();
}
