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
	QTextStream output(  fopen( "batch.returns", "w" ) , QIODevice::WriteOnly );	
	Test* object = new Test;
#include "batch.generated"
}

int main(int argc, char** argv)
{
	if ( argc > 1 ) {
		batch();
		return 0;
	}
	KCmdLineArgs::init( argc, argv, "TestApp", "Tests the dcop familly of tools + libraries", "1.0" ); // FIXME
	KApplication app;
	app.dcopClient()->attach(  );
	app.dcopClient()->registerAs( "TestApp" );
	new Test;
	return app.exec();
}
	
	
