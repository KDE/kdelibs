#include <kapp.h>
#include <ktmainwindow.h>
#include <qwidget.h>
#include <qtimer.h>
#include <stdlib.h>
#include "kled.h"
#include "kledtest.h"


int main( int argc, char **argv )
{
    KApplication a( argc, argv, "KLedTest" );
    KLedTest widget;
    // -----
    a.setMainWidget(&widget);
    widget.show();
    return a.exec();				// go
}

#include "kledtest.moc"

