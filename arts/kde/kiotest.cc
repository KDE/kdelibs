#include <stdio.h>
#include <kmedia2.h>
#include <connect.h>
#include <qapplication.h>
#include "qiomanager.h"
#include "artskde.h"

using namespace std;
using namespace Arts;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: testkio <url>\n");
		exit(1);
	}

	QApplication app(argc, argv);
	QIOManager qiomanager;
	Dispatcher dispatcher(&qiomanager);
	KIOInputStream stream;
	StdoutWriter writer;

	if(!stream.openURL(argv[1]))
	{
		printf("can't open file %s\n",argv[1]);
		exit(1);
	}

	connect(stream, writer);

	stream.start();
	writer.start();
	
	app.exec();
}
