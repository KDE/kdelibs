#include "ksqueezedtextlabel.h"
#include <kapplication.h>

int main( int argc, char **argv )
{
	KApplication app( argc, argv, "KSqueezedTextLabelTest" );

	KSqueezedTextLabel l( "This is a rather long string", 0);
	app.setMainWidget( &l );
	l.show();

	return app.exec();
}
