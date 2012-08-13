#include <QApplication>

#include "kcharselect.h"

int main (int argc,char **argv)
{
    QApplication::setApplicationName("kcharselecttest");

    QApplication app(argc, argv);

    KCharSelect selector( 0, 0 );
    selector.resize( selector.sizeHint() );
    selector.show();
    selector.setWindowTitle( "KCharSelect Test" );

    return app.exec();
}
