#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "addresslineedit.h"

using namespace KABC;

int main( int argc,char **argv )
{
  KAboutData aboutData( "testaddresslineedit",
                        I18N_NOOP( "Test Address LineEdit" ), "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  AddressLineEdit *lineEdit = new AddressLineEdit( 0 );

  lineEdit->show();
  app.setMainWidget( lineEdit );
  
  QObject::connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );

  app.exec();
  
  delete lineEdit;
}
