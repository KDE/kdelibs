#include "ksqueezedtextlabel.h"
#include <kapplication.h>
#include <kcmdlineargs.h>

int main( int argc, char **argv )
{
  KCmdLineArgs::init(argc, argv, "test", "Test", "test app", "1.0");
  KApplication app;

	KSqueezedTextLabel l( "This is a rather long string", 0);
	app.setMainWidget( &l );
	l.show();

	return app.exec();
}
