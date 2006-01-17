#include <QMenu>

#include <kcmdlineargs.h>
#include <kapplication.h>
#include <keditcl.h>

int main( int argc, char **argv )
{
  KCmdLineArgs::init( argc, argv, "test", "Test" ,"test app" ,"1.0" );
  KApplication app;
  KEdit *edit = new KEdit( 0L );
  QMenu *pop = new QMenu( 0L );
  pop->insertItem( "Popupmenu item" );
  edit->installRBPopup( pop );
  edit->show();
  return app.exec();
}
