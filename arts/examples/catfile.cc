#include <stdio.h>
#include <kmedia2.h>
#include <connect.h>

using namespace std;
using namespace Arts;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "usage: catfile <file>\n");
		exit(1);
	}

	Dispatcher dispatcher;
	FileInputStream file;
	StdoutWriter writer;

	if(!file.open(argv[1]))
	{
		printf("can't open file %s\n",argv[1]);
		exit(1);
	}

	connect(file, writer);

	file.start();
	writer.start();
	
	while(!file.eof())
		dispatcher.ioManager()->processOneEvent(false);
}
