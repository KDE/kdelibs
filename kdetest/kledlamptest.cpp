
#include <kapp.h>
#include <qwidget.h>
#include <qtimer.h>
#include <stdlib.h>
#include "kledlamp.h"


int main( int argc, char **argv )
{
    KApplication a( argc, argv );
	QWidget qw;
    QTimer t;
    KLedLamp l(&qw);				// create lamp

	t.start(1000, FALSE);
    QObject::connect(&t, SIGNAL(timeout()), &l, SLOT(toggle()));
	a.setMainWidget( &qw );
    qw.show();					// show widget
    return a.exec();				// go
}

