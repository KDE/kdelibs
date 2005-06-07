#include "test.h"
#include <kapplication.h>
#include <iostream>
#include <dcopclient.h>
#include <kcmdlineargs.h>



Test::~Test()
{
}

#include "definitions.generated"

using namespace std;

void batch()
{
	QTextStream output(  fopen( "batch.returns", "w" ) , IO_WriteOnly );	
	Test* object = new Test;
#include "batch.generated"
}

#ifdef Q_OS_WIN
# define main kdemain
#endif

int main(int argc, char** argv)
{
	if ( argc > 1 ) {
		batch();
		return 0;
	}
	KCmdLineArgs::init( argc, argv, "TestApp", "Tests the dcop familly of tools + libraries", "1.0" ); // FIXME
	KApplication app;
	if(!app.dcopClient()->attach(  ))
		return 1;

	app.dcopClient()->registerAs( "TestApp" );
	new Test;
	return app.exec();
}
	
	
