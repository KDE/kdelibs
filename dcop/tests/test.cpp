#include "test.h"
#include <qapplication.h>
#include <iostream>
#include <dcopclient.h>
#include <qtextstream.h>



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
	QApplication app( argc, argv );
        DCOPClient* dcopClient = new DCOPClient;
	dcopClient->attach();
	dcopClient->registerAs( "TestApp" );
	new Test;
	return app.exec();
}
	
	
