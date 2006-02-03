#include <QApplication>
#include <console.h>

int main ( int argc, char **argv )
{
	QApplication app( argc, argv );
	Console console;
	console.show();
	app.exec();
}
