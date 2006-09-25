#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "kdatetable.h"

int main( int argc, char** argv )
{
  KLocale::setMainCatalog( "kdelibs" );

  KCmdLineArgs::init( argc, argv, "test", "Test", "test app", "1.0" );
  KApplication app;

  KDateTable widget;
  widget.show();

  return app.exec();
}
