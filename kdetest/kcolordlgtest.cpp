
#include <qapp.h>
#include "kcolordlg.h"

int main( int argc, char *argv[] )
{
	QColor color;

	QApplication::setColorMode( QApplication::CustomColors );
	QApplication a( argc, argv );
	return KColorDialog::getColor( color );
}

