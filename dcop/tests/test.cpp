#include "test.h"
#include <qcoreapplication.h>
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

/*#ifdef Q_OS_WIN
# define main kdemain
#endif*/

int main(int argc, char** argv)
{
	if ( argc > 1 ) {
		batch();
		return 0;
	}
	QCoreApplication app( argc, argv );
        DCOPClient* dcopClient = new DCOPClient;
	if (!dcopClient->attach())
		return -1;
	dcopClient->registerAs( "TestApp" );
	new Test;
	return app.exec();
}
	
	
