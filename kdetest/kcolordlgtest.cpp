
#include <qapp.h>
#include "kcolordlg.h"
#include <Kconfig.h>

int main( int argc, char *argv[] )
{
	QColor color;
	KConfig aConfig;
	aConfig.setGroup( "KColorDialog-test" );

	QApplication::setColorMode( QApplication::CustomColors );
	QApplication a( argc, argv );

	color = aConfig.readColorEntry( "Chosen" );
	int nRet = KColorDialog::getColor( color );
	aConfig.writeEntry( "Chosen", color );
	
	return nRet;
}

